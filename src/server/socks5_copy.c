#include <socks5_copy.h>

void copy_read_handler(struct selector_key *key);
void copy_write_handler(struct selector_key *key);

const struct fd_handler copy_selector_handler = {
    .handle_read  = copy_read_handler,
    .handle_write = copy_write_handler,
   
};



void copy_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    if (selector_register(key->s, conn->origin_fd, &copy_selector_handler, OP_READ, key->data) != SELECTOR_SUCCESS) {
        return SOCKS5_ERROR;
    }

    return SOCKS5_COPY;
}


unsigned int copy_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    buffer *buff = NULL;
    int src_fd, dst_fd;
    bool from_client = (key->fd == conn->client_fd);

    if (from_client) {
        buff = &conn->in_buff;
        src_fd = conn->client_fd;
        dst_fd = conn->origin_fd;
    } else {
        buff = &conn->out_buff;
        src_fd = conn->origin_fd;
        dst_fd = conn->client_fd;
    }

    size_t max_write;
    uint8_t *write_ptr = buffer_write_ptr(buff, &max_write);

    ssize_t nread = recv(src_fd, write_ptr, max_write, MSG_DONTWAIT);

    if (nread <= 0) {
        return SOCKS5_DONE; 
    }

    buffer_write_adv(buff, nread);

    selector_status st = selector_set_interest(key->s, dst_fd, OP_WRITE);
    if (st != SELECTOR_SUCCESS) {
        return SOCKS5_ERROR;
    }

    if (buffer_can_write(buff)) {
        selector_set_interest(key->s, src_fd, OP_READ);
    } else {
        selector_set_interest(key->s, src_fd, OP_NOOP); // Desactivar lectura
    }

    return SOCKS5_COPY;
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
            return SOCKS5_ERROR;
        }
        return SOCKS5_COPY;
    }

    buffer_read_adv(buff, nwritten);

    if (!buffer_can_read(buff)) {
        selector_set_interest(key->s, dst_fd, OP_READ);
    } else {
        selector_set_interest(key->s, dst_fd, OP_WRITE);
    }

    return SOCKS5_COPY;
}

void copy_read_handler(struct selector_key *key) {
    copy_read(key);
}

void copy_write_handler(struct selector_key *key) {
    copy_write(key);
}