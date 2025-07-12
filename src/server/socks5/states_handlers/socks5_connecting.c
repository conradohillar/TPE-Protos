#include <socks5_connecting.h>

void handle_write_connecting(struct selector_key* key);

const struct fd_handler connecting_handler = {
    .handle_write = handle_write_connecting,
};

unsigned int connecting_on_block_ready(struct selector_key* key) {

    socks5_conn_t* conn = key->data;

    if (conn->addr_info == NULL) {
        LOG(ERROR, "No address info available for fd %d", key->fd);
        access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, SOCKS5_REP_HOST_UNREACHABLE, time(NULL));
        socks5_conn_req_response response = create_conn_req_error_response(SOCKS5_REP_HOST_UNREACHABLE);
        buffer_write_struct(&conn->out_buff, &response, SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV4_ADDR_SIZE);
        conn->is_error_response = true;
        selector_set_interest(key->s, conn->client_fd, OP_WRITE);
        return SOCKS5_CONNECTION_REQ;
    }

    int result = connect_to_host(&conn->addr_info, &conn->origin_fd);

    if (result == CONNECTION_IN_PROGRESS) {
        selector_set_interest(key->s, conn->client_fd, OP_NOOP);
        selector_status status = selector_register(key->s, conn->origin_fd, &connecting_handler, OP_WRITE, key->data);
        if (status != SELECTOR_SUCCESS) {
            LOG(ERROR, "Failed to register origin_fd %d for write: %s", conn->origin_fd, selector_error(status));
            access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, SOCKS5_REP_GENERAL_FAILURE, time(NULL));
            return SOCKS5_ERROR;
        }
        return SOCKS5_CONNECTING;
    } else if (result == CONNECTION_SUCCESS) {
        access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, SOCKS5_REP_SUCCEEDED, time(NULL));
        socks5_conn_req_response response = create_conn_req_response(SOCKS5_REP_SUCCEEDED, conn->origin_fd);
        size_t response_size = response.address_type == SOCKS5_CONN_REQ_ATYP_IPV4 ? SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV4_ADDR_SIZE : SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV6_ADDR_SIZE;
        buffer_write_struct(&conn->out_buff, &response, response_size);
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


unsigned int connecting_write(struct selector_key* key) {
     socks5_conn_t* conn = key->data;
    if(conn->is_error_response){
        return SOCKS5_ERROR;
    }else{
        return SOCKS5_COPY;
    }
}

uint8_t get_code(void) {
    switch (errno) {
        case ECONNREFUSED:
            return SOCKS5_REP_CONNECTION_REFUSED;
        case ENETUNREACH:
            return SOCKS5_REP_NETWORK_UNREACHABLE;
        case EHOSTUNREACH:
            return SOCKS5_REP_HOST_UNREACHABLE;
        case ETIMEDOUT:
            return SOCKS5_REP_TTL_EXPIRED;
        default:
            return SOCKS5_REP_HOST_UNREACHABLE;
    }

    return SOCKS5_REP_GENERAL_FAILURE;
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

    socks5_reply_status response_code = get_code();

    if (err != 0) {
        LOG(ERROR, "Connection error for fd %d: %s", conn->origin_fd, strerror(err));
        close(conn->origin_fd);

        if (conn->addr_info != NULL) {
            int result = connect_to_host(&conn->addr_info, &conn->origin_fd);
            if (result == CONNECTION_IN_PROGRESS) {
                LOG(INFO, "Connection to destination for fd %d is in progress", key->fd);
                return ;
            } else if (result == CONNECTION_FAILED) {
                LOG(ERROR, "Failed to connect to destination for fd %d", key->fd);
                access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, response_code, time(NULL));
                selector_unregister_fd(key->s, conn->origin_fd);
                close(conn->origin_fd);
                conn->origin_fd = -1;
                selector_unregister_fd(key->s, conn->client_fd);
                return ;
            }
        } else {
            LOG(ERROR, "No address info available for fd %d", key->fd);
            access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, response_code, time(NULL));
            socks5_conn_req_response response = create_conn_req_error_response(response_code);
            buffer_write_struct(&conn->out_buff, &response, SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV4_ADDR_SIZE);
            conn->is_error_response = true;
            selector_unregister_fd(key->s, conn->origin_fd);
            close(conn->origin_fd);
            conn->origin_fd = -1;
            selector_set_interest(key->s, conn->client_fd, OP_WRITE);
            return ;
        }
    }

    freeaddrinfo(conn->addr_info);
    selector_unregister_fd(key->s, conn->origin_fd);
    stm_handler_read(conn->stm, key);
   
    access_register_add_entry(get_server_data()->access_register, conn->username, conn->src_address, conn->src_port, conn->dst_address, conn->dst_port, SOCKS5_REP_SUCCEEDED, time(NULL));
    socks5_conn_req_response response = create_conn_req_response(SOCKS5_REP_SUCCEEDED, conn->origin_fd);
    size_t response_size = response.address_type == SOCKS5_CONN_REQ_ATYP_IPV4 ? SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV4_ADDR_SIZE : SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV6_ADDR_SIZE;
    buffer_write_struct(&conn->out_buff, &response, response_size);
    selector_set_interest(key->s, conn->client_fd, OP_WRITE);
    buffer_reset(&conn->in_buff);
}
