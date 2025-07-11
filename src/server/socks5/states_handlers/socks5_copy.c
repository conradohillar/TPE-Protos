#include <socks5_copy.h>
#include <errno.h>
#include <string.h>

void copy_read_handler(struct selector_key* key);
void copy_write_handler(struct selector_key* key);
void copy_close_handler(struct selector_key* key);

const struct fd_handler copy_selector_handler = {
    .handle_read = copy_read_handler,
    .handle_write = copy_write_handler,
    .handle_close = copy_close_handler,
};

void copy_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    LOG(INFO, "Starting copy phase for fd %d (client) -> fd %d (origin)", conn->client_fd, conn->origin_fd);

    if (selector_register(key->s, conn->origin_fd, &copy_selector_handler, OP_NOOP, key->data) != SELECTOR_SUCCESS) {
        LOG(ERROR, "Failed to register origin fd %d for copy phase", conn->origin_fd);
    }
}

// el fd quiere escribir algo por eso el socket se "levanta" con interes de lectura
unsigned int copy_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    if (selector_set_interest(key->s, conn->origin_fd, OP_WRITE)) {
        LOG(ERROR, "Failed to set interest for origin fd %d in copy phase", conn->origin_fd);
        return SOCKS5_ERROR;
    }
    return SOCKS5_COPY;
}

void copy_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    LOG(INFO, "Ending copy phase for fd %d (client) -> fd %d (origin)", conn->client_fd, conn->origin_fd);

    if (selector_unregister_fd(key->s, conn->origin_fd) != SELECTOR_SUCCESS) {
        LOG(ERROR, "Failed to unregister origin fd %d in copy phase", conn->origin_fd);
    }
}

void copy_read_handler(struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    if (!buffer_can_write(&conn->out_buff)) {
        LOG(WARNING, "Output buffer full for fd %d, setting NOOP", key->fd);
        selector_set_interest(key->s, conn->origin_fd, OP_NOOP); // Por ahora no hago nada, hago que quede el socket muerto y despues vemos igual no deberia pasar este caso creo
        return;
    }

    size_t n;
    uint8_t* write_ptr = buffer_write_ptr(&conn->out_buff, &n);

    ssize_t n_read = recv(conn->origin_fd, write_ptr, n, MSG_DONTWAIT);

    if (n_read > 0) {
        LOG(DEBUG, "Read %zd bytes from fd %d", n_read, conn->origin_fd);
        buffer_write_adv(&conn->out_buff, n_read);
        selector_set_interest(key->s, conn->client_fd, OP_WRITE);
    } else if (n_read == 0) {
        LOG(INFO, "Connection closed by dest on fd %d", conn->origin_fd);
        selector_unregister_fd(key->s, conn->origin_fd);
        selector_unregister_fd(key->s, conn->client_fd);
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG(ERROR, "Error reading from fd %d: %s", conn->origin_fd, strerror(errno));
            selector_unregister_fd(key->s, conn->origin_fd);
            selector_unregister_fd(key->s, conn->client_fd);
        }
    }
    // LEER DEL FD DESTINO Y PONERLO EN EL BUFFER DE OUT SETEA EL FD DEL CLIENTE EN OP_WRITE
}

void copy_write_handler(struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    if (!buffer_can_read(&conn->in_buff)) {
        LOG(DEBUG, "Output buffer empty for fd %d, setting NOOP", key->fd);
        selector_set_interest(key->s, conn->origin_fd, OP_NOOP); // No hay nada que enviar, no hacemos nada
        return;
    }

    bool full_buffer = !buffer_can_write(&conn->in_buff);

    size_t n;
    uint8_t* read_ptr = buffer_read_ptr(&conn->in_buff, &n);

    ssize_t n_written = send(conn->origin_fd, read_ptr, n, MSG_DONTWAIT);
    if (n_written > 0) {
        LOG(DEBUG, "Wrote %zd bytes to fd %d", n_written, conn->origin_fd);
        buffer_read_adv(&conn->in_buff, n_written); // Avanzamos el puntero de lectura del buffer de salida
        if (!buffer_can_read(&conn->in_buff)) {
            selector_set_interest(key->s, conn->origin_fd, OP_READ); // Si ya no hay nada que enviar, ponemos el fd en NOOP
        }
        if (full_buffer) {
            selector_set_interest(key->s, conn->client_fd, OP_READ);
        }

    } else if (n_written == 0) {
        LOG(INFO, "Connection closed by dest on fd %d", conn->origin_fd);
        selector_unregister_fd(key->s, conn->origin_fd);
        selector_unregister_fd(key->s, conn->client_fd);

        // veamos aca que hacer, por ahora quedan los dos fd muertos
        //  ACA RECIBIMOS EOF, CREO QUE DEBERIAMOS LIBERAR LOS RECURSOS
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG(ERROR, "Error writing to fd %d: %s", conn->origin_fd, strerror(errno));
            // aca creo que tenemos que librerar los recursos
        }
    }

}


void copy_close_handler(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    if(conn->origin_fd > 0) {
        LOG(DEBUG, "Closing origin connection fd %d", conn->origin_fd);
        close(conn->origin_fd);
        conn->origin_fd = -1;
    }
}
