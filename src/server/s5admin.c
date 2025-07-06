#include <s5admin.h>

// // Estructura para manejar el estado de cada conexión admin
typedef struct {
    int fd;
    char inbuf[MAX_CMD_LEN];
    size_t inbuf_len;
    char outbuf[MAX_RESPONSE_LEN];
    size_t outbuf_len;
    size_t outbuf_sent;
} admin_conn_t;


static void admin_read(struct selector_key *key);
static void admin_write(struct selector_key *key);

static const fd_handler admin_handler = {
    .handle_read = admin_read,
    .handle_write = admin_write,
    .handle_close = NULL,
    .handle_block = NULL
};

// Handler para aceptar conexiones
void s5admin_passive_accept(struct selector_key *key) {
    admin_conn_t *conn = calloc(1, sizeof(admin_conn_t));
    if (conn == NULL) {
        log_error("Error allocating memory for admin connection");
        perror("Error al asignar memoria para la conexión admin");
        return;
    }
    int fd = passive_accept(key, conn, &admin_handler);
    if(fd < 0) {
        log_error("Failed to accept admin connection");
        free(conn);
        return;
    }
    conn->fd = fd;
    log_info("Admin client connected on fd %d", fd);
}

static void admin_read(struct selector_key *key) {
    admin_conn_t *conn = key->data;
    ssize_t n = read(conn->fd, conn->inbuf + conn->inbuf_len, sizeof(conn->inbuf) - conn->inbuf_len - 1);
    if (n <= 0) {
        log_info("Admin connection closed on fd %d", conn->fd);
        selector_unregister_fd(key->s, conn->fd);
        close(conn->fd);
        free(conn);
        return;
    }
    log_debug("Read %zd bytes from admin connection fd %d", n, conn->fd);
    conn->inbuf_len += n;
    conn->inbuf[conn->inbuf_len] = '\0';

    char *newline = strchr(conn->inbuf, '\n');
    if (newline) {
        *newline = '\0';
        log_debug("Processing admin command: %s", conn->inbuf);
        conn->outbuf_len = config_handler(conn->inbuf, conn->outbuf, sizeof(conn->outbuf));
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
            log_error("Error writing to admin connection fd %d: %s", conn->fd, strerror(errno));
            selector_unregister_fd(key->s, conn->fd);
            close(conn->fd);
            free(conn);
            return;
        }
        log_debug("Wrote %zd bytes to admin connection fd %d", n, conn->fd);
        conn->outbuf_sent += n;
    }
    // Terminó de escribir, volver a leer
    selector_set_interest_key(key, OP_READ);
}

