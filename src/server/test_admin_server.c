#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "selector.h"
#include "config.h"

#define DEFAULT_PORT 9090
#define BACKLOG 5

// Estructura para manejar el estado de cada conexión admin
typedef struct {
    int fd;
    char inbuf[MAX_CMD_LEN];
    size_t inbuf_len;
    char outbuf[MAX_RESPONSE_LEN];
    size_t outbuf_len;
    size_t outbuf_sent;
} admin_conn_t;

static void accept_handler(struct selector_key *key);
static void admin_read(struct selector_key *key);
static void admin_write(struct selector_key *key);

static access_register_t *access_register = NULL;

static const fd_handler admin_handler = {
    .handle_read = admin_read,
    .handle_write = admin_write,
    .handle_close = NULL,
    .handle_block = NULL
};

static void admin_read(struct selector_key *key) {
    admin_conn_t *conn = key->data;
    ssize_t n = read(conn->fd, conn->inbuf + conn->inbuf_len, sizeof(conn->inbuf) - conn->inbuf_len - 1);
    if (n <= 0) {
        selector_unregister_fd(key->s, conn->fd);
        close(conn->fd);
        free(conn);
        return;
    }
    conn->inbuf_len += n;
    conn->inbuf[conn->inbuf_len] = '\0';

    char *newline = strchr(conn->inbuf, '\n');
    if (newline) {
        *newline = '\0';
        conn->outbuf_len = config_handler(conn->inbuf, conn->outbuf, sizeof(conn->outbuf), access_register);
        conn->outbuf_sent = 0;
        selector_set_interest_key(key, OP_WRITE);
        // Mover datos restantes (si hay más de un comando en el buffer)
        size_t rem = conn->inbuf_len - (newline - conn->inbuf + 1);
        memmove(conn->inbuf, newline + 1, rem);
        conn->inbuf_len = rem;
    }
}

static void admin_write(struct selector_key *key) {
    admin_conn_t *conn = key->data;
    while (conn->outbuf_sent < conn->outbuf_len) {
        ssize_t n = write(conn->fd, conn->outbuf + conn->outbuf_sent, conn->outbuf_len - conn->outbuf_sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            selector_unregister_fd(key->s, conn->fd);
            close(conn->fd);
            free(conn);
            return;
        }
        conn->outbuf_sent += n;
    }
    // Terminó de escribir, volver a leer
    selector_set_interest_key(key, OP_READ);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) port = atoi(argv[++i]);
    }
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) { perror("socket"); exit(1); }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind"); close(listenfd); exit(1);
    }
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen"); close(listenfd); exit(1);
    }
    selector_fd_set_nio(listenfd);
    printf("Servidor admin multiplexado escuchando en puerto %d...\n", port);

    access_register = access_register_init();
    if (!access_register) {
        close(listenfd);
        exit(1);
    }

    struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = { .tv_sec = 10, .tv_nsec = 0 }
    };
    if (selector_init(&conf) != SELECTOR_SUCCESS) {
        fprintf(stderr, "Error inicializando selector\n");
        exit(1);
    }
    fd_selector selector = selector_new(1024);
    if (!selector) {
        fprintf(stderr, "Error creando selector\n");
        exit(1);
    }

    
    static const fd_handler acceptor_handler = {
        .handle_read = accept_handler,
        .handle_write = NULL,
        .handle_close = NULL,
        .handle_block = NULL
    };
    selector_register(selector, listenfd, &acceptor_handler, OP_READ, NULL);

    // Loop principal
    while (selector_select(selector) == SELECTOR_SUCCESS);
        
    selector_destroy(selector);
    selector_close();
    close(listenfd);
    free(access_register);
    return 0;
}

// Handler para aceptar conexiones
static void accept_handler(struct selector_key *key) {
    int listenfd = key->fd;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (connfd < 0) return;
    selector_fd_set_nio(connfd);
    admin_conn_t *conn = calloc(1, sizeof(admin_conn_t));
    conn->fd = connfd;
    selector_register(key->s, connfd, &admin_handler, OP_READ, conn);
    printf("Cliente admin conectado!\n");
}