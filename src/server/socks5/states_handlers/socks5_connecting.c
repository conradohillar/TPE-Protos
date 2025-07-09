#include <socks5_connecting.h>

void handle_write_connecting(struct selector_key* key);

const struct fd_handler connecting_handler = {
    .handle_write = handle_write_connecting,
};

unsigned int connecting_on_block_ready(struct selector_key* key) {

    socks5_conn_t* conn = key->data;

    if (conn->addr_info == NULL) {
        // TENEMOS QUE MANDAR UNA RESPUESTA DE ERROR USANDO SOCKS5(NO SE PUDO RESOLVER EL NOMBRE DEL HOST)
        LOG(ERROR, "Address info is NULL for fd %d", key->fd);
        return SOCKS5_ERROR; // No se puede continuar sin la información de la dirección
    }

    int result = connect_to_host(&conn->addr_info, &conn->origin_fd);

    if (result == CONNECTION_IN_PROGRESS) {
        selector_set_interest(key->s, conn->client_fd, OP_NOOP);
        selector_status status = selector_register(key->s, conn->origin_fd, &connecting_handler, OP_WRITE, key->data);
        if (status != SELECTOR_SUCCESS) {
            LOG(ERROR, "Failed to register origin_fd %d for write: %s", conn->origin_fd, selector_error(status));
            close(conn->origin_fd);
            return SOCKS5_ERROR;
        }
        return SOCKS5_CONNECTING;
    } else if (result == CONNECTION_SUCCESS) {
        // TODO esto deberia ser una funcion que manda la respuesta.
        buffer_write(&conn->out_buff, SOCKS5_VERSION);
        buffer_write(&conn->out_buff, SOCKS5_SUCCESS);
        buffer_write(&conn->out_buff, SOCKS5_CONN_REQ_RSV); // RSV
        buffer_write(&conn->out_buff, ATYP_IPV4);           // ESTO LO DEBERIAMOS TENER GUARDADO

        // pongo una ip a mano para probar
        buffer_write(&conn->out_buff, 0x7F); // IP
        buffer_write(&conn->out_buff, 0x00); // IP
        buffer_write(&conn->out_buff, 0x00); // IP
        buffer_write(&conn->out_buff, 0x01); // IP
        buffer_write(&conn->out_buff, (conn->conn_req_parser->dst_port >> 8) & 0xFF);
        buffer_write(&conn->out_buff, conn->conn_req_parser->dst_port & 0xFF);

        selector_set_interest_key(key, OP_WRITE);
        buffer_reset(&conn->in_buff);
        return SOCKS5_COPY;
    } else {
        return SOCKS5_ERROR;
    }
}

unsigned int connecting_read(struct selector_key* key) {
    return SOCKS5_COPY;
}

void handle_write_connecting(struct selector_key* key) {
    LOG(DEBUG, "Handling write for connecting state, fd: %d", key->fd);
    socks5_conn_t* conn = key->data;

    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(conn->origin_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        LOG(ERROR, "getsockopt failed for fd %d: %s", conn->origin_fd, strerror(errno));
        close(conn->origin_fd);
    }

    if (err != 0) {
        LOG(ERROR, "Connection error for fd %d: %s", conn->origin_fd, strerror(err));
        close(conn->origin_fd);

        if (conn->addr_info != NULL) {
            int result = connect_to_host(&conn->addr_info, &conn->origin_fd);
            if (result == CONNECTION_IN_PROGRESS) {
                LOG(INFO, "Connection to destination for fd %d is in progress", key->fd);
            } else if (result == CONNECTION_SUCCESS) {
                // TODO esto deberia ser una funcion que manda la respuesta.
                LOG(INFO, "Successfully connected to destination for fd %d, origin_fd: %d", key->fd, conn->origin_fd);
                buffer_write(&conn->out_buff, SOCKS5_VERSION);
                buffer_write(&conn->out_buff, SOCKS5_SUCCESS);
                buffer_write(&conn->out_buff, SOCKS5_CONN_REQ_RSV); // RSV
                buffer_write(&conn->out_buff, ATYP_IPV4);           // ESTO LO DEBERIAMOS TENER GUARDADO

                // pongo una ip a mano para probar
                buffer_write(&conn->out_buff, 0x7F); // IP
                buffer_write(&conn->out_buff, 0x00); // IP
                buffer_write(&conn->out_buff, 0x00); // IP
                buffer_write(&conn->out_buff, 0x01); // IP
                buffer_write(&conn->out_buff, (conn->conn_req_parser->dst_port >> 8) & 0xFF);
                buffer_write(&conn->out_buff, conn->conn_req_parser->dst_port & 0xFF);

                selector_unregister_fd(key->s, conn->origin_fd);
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&conn->in_buff);
            } else {
                LOG(ERROR, "Failed to connect to destination for fd %d", key->fd);
                selector_unregister_fd(key->s, conn->origin_fd);
                close(conn->origin_fd);
                selector_unregister_fd(key->s, conn->client_fd);
            }
        }
    }

    freeaddrinfo(conn->addr_info);

    selector_unregister_fd(key->s, conn->origin_fd);
    stm_handler_read(conn->stm, key);
    // TODO esto deberia ser una funcion que manda la respuesta.
    buffer_write(&conn->out_buff, SOCKS5_VERSION);
    buffer_write(&conn->out_buff, SOCKS5_SUCCESS);
    buffer_write(&conn->out_buff, SOCKS5_CONN_REQ_RSV); // RSV
    buffer_write(&conn->out_buff, ATYP_IPV4);           // ESTO LO DEBERIAMOS TENER GUARDADO

    // pongo una ip a mano para probar
    buffer_write(&conn->out_buff, 0x7F); // IP
    buffer_write(&conn->out_buff, 0x00); // IP
    buffer_write(&conn->out_buff, 0x00); // IP
    buffer_write(&conn->out_buff, 0x01); // IP
    buffer_write(&conn->out_buff, (conn->dst_port >> 8) & 0xFF);
    buffer_write(&conn->out_buff, conn->dst_port & 0xFF);

    selector_set_interest(key->s, conn->client_fd, OP_WRITE);
    buffer_reset(&conn->in_buff);
}