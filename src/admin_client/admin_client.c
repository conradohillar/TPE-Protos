#include "include/admin_client.h"

static void stdin_handler(struct selector_key *key);
static void sock_read_handler(struct selector_key *key);
static void sock_write_handler(struct selector_key *key);

static admin_client_state_t *global_state = NULL;
static fd_selector selector = NULL;

static const fd_handler stdin_fd_handler = {
    .handle_read = stdin_handler,
    .handle_write = NULL,
    .handle_close = NULL,
    .handle_block = NULL
};

static const fd_handler sock_fd_handler = {
    .handle_read = sock_read_handler,
    .handle_write = sock_write_handler,
    .handle_close = NULL,
    .handle_block = NULL
};

// connect no bloqueante
int connect_to_admin_server(const char *host, int port) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        struct hostent *he = gethostbyname(host);
        if (!he) { close(sockfd); return -1; }
        memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }
    int res = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static void stdin_handler(struct selector_key *key) {
    admin_client_state_t *state = global_state;
    char cmd[MAX_CMD_LEN];
    if (!fgets(cmd, sizeof(cmd), stdin)) {
        selector_unregister_fd(key->s, STDIN_FILENO);
        return;
    }
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len-1] == '\n') len--;
    if (state->outbuf_len - state->outbuf_sent + len + 1 > sizeof(state->outbuf)) {
        printf("Comando demasiado largo o buffer ocupado\n");
        return;
    }
    memcpy(state->outbuf, cmd, len);
    state->outbuf[len] = '\n';
    state->outbuf_len = len + 1;
    state->outbuf_sent = 0;
    selector_set_interest(selector, state->sockfd, OP_WRITE | OP_READ);
}

static void sock_read_handler(struct selector_key *key) {
    admin_client_state_t *state = global_state;
    ssize_t n = read(state->sockfd, state->inbuf + state->inbuf_len, sizeof(state->inbuf) - state->inbuf_len - 1);
    if (n <= 0) {
        printf("\n[Conexión cerrada por el servidor]\n");
        selector_unregister_fd(key->s, state->sockfd);
        selector_unregister_fd(key->s, STDIN_FILENO);
        release_resources(state);
        exit(0);
    }
    state->inbuf_len += n;
    state->inbuf[state->inbuf_len] = '\0';
    char *line = NULL;
    while ((line = strchr(state->inbuf, '\n'))) {
        *line = '\0';
        printf("%s\n", state->inbuf);
        if(strcmp(state->inbuf, "BYE") == 0){
            selector_unregister_fd(key->s, state->sockfd);
            selector_unregister_fd(key->s, STDIN_FILENO);
            release_resources(state);
            exit(0);
        }
        size_t rem = state->inbuf_len - (line - state->inbuf + 1);
        memmove(state->inbuf, line + 1, rem);
        state->inbuf_len = rem;
        state->inbuf[state->inbuf_len] = '\0';
    }
    if(state->inbuf_len == 0){
        printf(">: ");
        fflush(stdout);
    }
}

static void sock_write_handler(struct selector_key *key) {
    admin_client_state_t *state = global_state;
    // Handshake de connect no bloqueante
    if (!state->connected) {
        int err = 0; socklen_t len = sizeof(err);
        if (getsockopt(state->sockfd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
            printf("No se pudo conectar\n");
            selector_unregister_fd(key->s, state->sockfd);
            selector_unregister_fd(key->s, STDIN_FILENO);
            release_resources(state);
            exit(1);
        }
        state->connected = 1;
        printf("[Conectado al servidor de administración]\n>: ");
        fflush(stdout);
        selector_set_interest(selector, state->sockfd, OP_READ);
        return;
    }
    if (state->outbuf_len > state->outbuf_sent) {
        ssize_t n = write(state->sockfd, state->outbuf + state->outbuf_sent, state->outbuf_len - state->outbuf_sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            printf("Error enviando comando\n");
            selector_unregister_fd(key->s, state->sockfd);
            selector_unregister_fd(key->s, STDIN_FILENO);
            release_resources(state);
            exit(1);
        }
        state->outbuf_sent += n;
        if (state->outbuf_sent == state->outbuf_len) {
            state->outbuf_len = 0;
            state->outbuf_sent = 0;
            selector_set_interest(selector, state->sockfd, OP_READ);
        }
    }
}

void admin_client_loop(int sockfd) {
    admin_client_state_t *state = calloc(1, sizeof(admin_client_state_t));
    state->sockfd = sockfd;
    state->connected = 0;
    global_state = state;
    struct selector_init conf = {
        .signal = SIGUSR1,
        .select_timeout = { .tv_sec = 10, .tv_nsec = 0 }
    };
    if (selector_init(&conf) != SELECTOR_SUCCESS) {
        fprintf(stderr, "Error inicializando selector\n");
        exit(1);
    }
    selector = selector_new(16);
    if (!selector) {
        fprintf(stderr, "Error creando selector\n");
        exit(1);
    }
    selector_register(selector, sockfd, &sock_fd_handler, OP_WRITE, NULL); // handshake connect
    selector_register(selector, STDIN_FILENO, &stdin_fd_handler, OP_READ, NULL);
    
    // loop principal
    while (selector_select(selector) == SELECTOR_SUCCESS);

    release_resources(state);
}

int main(int argc, char *argv[]) {
    const char *host = DEFAULT_SERVER_HOST;
    int port = DEFAULT_SERVER_PORT;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) port = atoi(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0 && i+1 < argc) host = argv[++i];
    }
    int sockfd = connect_to_admin_server(host, port);
    if (sockfd < 0) {
        fprintf(stderr, "No se pudo conectar a %s:%d\n", host, port);
        return 1;
    }
    admin_client_loop(sockfd);
    return 0;
}

void release_resources(admin_client_state_t *state){
    selector_destroy(selector);
    selector_close();
    close(state->sockfd);
    free(state);
}
