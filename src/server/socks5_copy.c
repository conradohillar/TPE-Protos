#include <socks5_copy.h>
#include <errno.h>
#include <string.h>

void copy_read_handler(struct selector_key *key);
void copy_write_handler(struct selector_key *key);

const struct fd_handler copy_selector_handler = {
    .handle_read  = copy_read_handler,
    .handle_write = copy_write_handler,
};

void copy_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    log_info("Starting copy phase for fd %d (client) -> fd %d (origin)", conn->client_fd, conn->origin_fd);

    if (selector_register(key->s, conn->origin_fd, &copy_selector_handler, OP_READ, key->data) != SELECTOR_SUCCESS) {
        log_error("Failed to register origin fd %d for copy phase", conn->origin_fd);
    }
}

//el fd quiere escribir algo por eso el socket se "levanta" con interes de lectura
unsigned int copy_read(struct selector_key *key) {
    //PONER EL FD DESTINO CON INTERES DE ESCRITURA
}

unsigned int copy_write(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    buffer *buff = NULL;
    int dst_fd = key->fd;
    bool to_client = (dst_fd == conn->client_fd);

    buff = to_client ? &conn->out_buff : &conn->in_buff;

    size_t n;
    uint8_t *read_ptr = buffer_read_ptr(buff, &n);

    ssize_t nwritten = send(dst_fd, read_ptr, n, MSG_DONTWAIT);
    if (nwritten < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_error("Error writing in copy phase for fd %d: %s", dst_fd, strerror(errno));
            return SOCKS5_ERROR;
        }
        return SOCKS5_COPY;
    }

    log_debug("Copied %zd bytes to fd %d (to_client: %s)", nwritten, dst_fd, to_client ? "true" : "false");
    buffer_read_adv(buff, nwritten);

    if (!buffer_can_read(buff)) {
        selector_set_interest(key->s, dst_fd, OP_NOOP);

        int src_fd = to_client ? conn->origin_fd : conn->client_fd;
        selector_set_interest(key->s, src_fd, OP_READ);
    } else {
        selector_set_interest(key->s, dst_fd, OP_WRITE);
    }

    return SOCKS5_COPY;
}

void copy_read_handler(struct selector_key *key) {
    //LEER DEL FD DESTINO Y PONERLO EN EL BUFFER DE OUT SETEA EL FD DEL CLIENTE EN OP_WRITE
}

void copy_write_handler(struct selector_key *key) {
    //ENVIA LO QUE HABIA EN IN A FD_DEST Y SETEA ESE FD EN OP_READ
}
