/**
 * socks5.c - accepta conexiones y controla el flujo de un proxy SOCKSv5
 * (sockets no bloqueantes)
 */

#include <socks5.h>

/* declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void socksv5_read(struct selector_key* key);
static void socksv5_write(struct selector_key* key);
static void socksv5_block(struct selector_key* key);
static void socksv5_close(struct selector_key* key);

static const struct fd_handler socks5_handler = {
    .handle_read = socksv5_read,
    .handle_write = socksv5_write,
    .handle_close = socksv5_close,
    .handle_block = socksv5_block,
};

void socksv5_passive_accept(struct selector_key* key) {
    LOG_MSG(DEBUG, "Trying to accept a new SOCKSv5 connection");
    if(get_server_data()->metrics->current_connections >= get_server_data()->max_conn) {
        LOG_MSG(WARNING, "Max connections reached, rejecting new connection");
        // Se acepta y cierra inmediatamente para notificar al cliente
        // que se rechaza la conexión.
        int fd = accept(key->fd, NULL, NULL);
        if (fd != -1) close(fd);
        return;
    }

    socks5_conn_t* conn = malloc(sizeof(socks5_conn_t));
    if (conn == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for SOCKSv5 connection");
        return;
    }
    key->data = conn;

    int fd = passive_accept(key, conn, &socks5_handler);
    if (fd == -1) {
        LOG_MSG(ERROR, "Failed to accept SOCKSv5 connection");
        free(conn);
        return;
    }

    conn->s = key->s;
    conn->client_fd = fd;
    conn->origin_fd = 0;
    conn->in_buff_data = malloc(get_server_data()->buffer_size);
    if(conn->in_buff_data == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for input buffer data");
        selector_unregister_fd(key->s, fd);
        return;
    }
    conn->out_buff_data = malloc(get_server_data()->buffer_size);
    if(conn->out_buff_data == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for output buffer data");
        selector_unregister_fd(key->s, fd);
        return;
    }   
    buffer_init(&conn->in_buff, SOCKS5_BUFF_MAX_LEN, conn->in_buff_data);
    buffer_init(&conn->out_buff, SOCKS5_BUFF_MAX_LEN, conn->out_buff_data);
    
    conn->stm = socks5_stm_init();
    if (conn->stm == NULL) {
        LOG_MSG(ERROR, "Failed to initialize SOCKSv5 state machine");
        selector_unregister_fd(key->s, fd);
        return;
    }
   
    LOG(INFO, "New SOCKSv5 connection accepted on fd %d", fd);

    metrics_inc_curr_conn(get_server_data()->metrics);
    metrics_inc_total_conn(get_server_data()->metrics);
}

static void socksv5_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    if (!buffer_can_write(&conn->in_buff)) {
        LOG(WARNING, "Input buffer full for fd %d, setting NOOP", key->fd);
        selector_set_interest_key(key, OP_NOOP);
        if (conn->origin_fd != 0) { selector_set_interest(key->s, conn->origin_fd, OP_WRITE); }
        return;
    }

    size_t n;
    uint8_t* write_ptr = buffer_write_ptr(&conn->in_buff, &n);

    ssize_t n_read = recv(key->fd, write_ptr, n, MSG_DONTWAIT);

    if (n_read > 0) {
        LOG(DEBUG, "Read %zd bytes from fd %d", n_read, key->fd);
        buffer_write_adv(&conn->in_buff, n_read);
        socks5_state state = stm_handler_read(conn->stm, key);
        if (state == SOCKS5_ERROR) {
            LOG(ERROR, "Error in SOCKS5 state machine for fd %d", key->fd);
            metrics_inc_errors(get_server_data()->metrics);
        }

    } else if (n_read == 0) {
        LOG(INFO, "Connection closed by client on fd %d", key->fd);
        if(conn->origin_fd > 0) {
            selector_unregister_fd(key->s, conn->origin_fd);
        }
        selector_unregister_fd(key->s, conn->client_fd);
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG(ERROR, "Error reading from fd %d: %s", key->fd, strerror(errno));
            metrics_inc_errors(get_server_data()->metrics);
            if(conn->origin_fd > 0) {
                selector_unregister_fd(key->s, conn->origin_fd);
            }
            selector_unregister_fd(key->s, conn->client_fd);
        }
    }
}

static void socksv5_write(struct selector_key* key) {
    socks5_conn_t* conn = key->data;

    if (!buffer_can_read(&conn->out_buff)) {
        LOG(DEBUG, "Output buffer empty for fd %d, setting NOOP", key->fd);
        selector_set_interest_key(key, OP_NOOP);
        return;
    }

    size_t n;
    uint8_t* read_ptr = buffer_read_ptr(&conn->out_buff, &n);

    ssize_t n_written = send(key->fd, read_ptr, n, MSG_DONTWAIT);

    if (n_written > 0) {
        LOG(DEBUG, "Wrote %zd bytes to fd %d", n_written, key->fd);
        metrics_add_bytes(get_server_data()->metrics, n_written);
        buffer_read_adv(&conn->out_buff, n_written);
        if (!buffer_can_read(&conn->out_buff)) {
            selector_set_interest_key(key, OP_READ);
            buffer_reset(&conn->out_buff);
        }
        if (buffer_can_write(&conn->in_buff)) { selector_set_interest_key(key, OP_READ); }
        if(has_write_handler(conn->stm->current->state)){
            stm_handler_write(conn->stm, key);
        }
    } else if (n_written < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG(ERROR, "Error writing to fd %d: %s", key->fd, strerror(errno));
            metrics_inc_errors(get_server_data()->metrics);
            if(conn->origin_fd > 0) {
                selector_unregister_fd(key->s, conn->origin_fd);
            }
            selector_unregister_fd(key->s, conn->client_fd);

        }
    }
}

static void socksv5_block(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Handling block event for fd %d", key->fd);
    socks5_state state = stm_handler_block(conn->stm, key);
    if (state == SOCKS5_ERROR) {
        LOG(ERROR, "Error in SOCKS5 state machine block handler for fd %d", key->fd);
        metrics_inc_errors(get_server_data()->metrics);
                    selector_unregister_fd(key->s, conn->client_fd);

    }
}

static void socksv5_close(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(INFO, "Closing SOCKS5 connection on fd %d", key->fd);
    if (conn->client_fd > 0) {
        LOG(DEBUG, "Closing client connection fd %d", conn->client_fd);
        close(conn->client_fd);
        conn->client_fd = -1;
    }
    socks5_stm_free(conn->stm);
    metrics_dec_curr_conn(get_server_data()->metrics);
    free(conn->in_buff_data);
    free(conn->out_buff_data);
    free(conn);
}