#include <conn_req.h>


void connection_req_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Starting connection request phase for fd %d", key->fd);
    conn->conn_req_parser = conn_req_parser_init();
}

unsigned int connection_req_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {

        conn_req_parser_state state = conn_req_parser_feed(conn->conn_req_parser, buffer_read(&conn->in_buff));

        if (state == CONN_REQ_DONE) {
            LOG(INFO, "Connection request received for fd %d, connecting to destination", key->fd);
            
            if(conn->conn_req_parser->cmd != SOCKS5_CONN_REQ_CMD_CONNECT) {
                LOG(ERROR, "Unsupported command %d for fd %d", conn->conn_req_parser->cmd, key->fd);
                socks5_conn_req_response response = create_conn_req_error_response(SOCKS5_REP_COMMAND_NOT_SUPPORTED);
                buffer_write_struct(&conn->out_buff, &response, SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE + IPV4_ADDR_SIZE);
                conn->is_error_response = true;
                selector_set_interest(key->s, conn->client_fd, OP_WRITE);
                return SOCKS5_CONNECTION_REQ;
            }
            
            selector_set_interest_key(key, OP_NOOP);

            conn->dst_port = conn->conn_req_parser->dst_port;
            if(conn->conn_req_parser->atyp == SOCKS5_CONN_REQ_ATYP_IPV4) {
                snprintf(conn->dst_address, sizeof(conn->dst_address), "%d.%d.%d.%d",
                         conn->conn_req_parser->dst_addr[0],
                         conn->conn_req_parser->dst_addr[1],
                         conn->conn_req_parser->dst_addr[2],
                         conn->conn_req_parser->dst_addr[3]);
            } else if(conn->conn_req_parser->atyp == SOCKS5_CONN_REQ_ATYP_IPV6) {
                snprintf(conn->dst_address, sizeof(conn->dst_address), "%x:%x:%x:%x:%x:%x:%x:%x",
                         (conn->conn_req_parser->dst_addr[0] << 8) | conn->conn_req_parser->dst_addr[1],
                         (conn->conn_req_parser->dst_addr[2] << 8) | conn->conn_req_parser->dst_addr[3],
                         (conn->conn_req_parser->dst_addr[4] << 8) | conn->conn_req_parser->dst_addr[5],
                         (conn->conn_req_parser->dst_addr[6] << 8) | conn->conn_req_parser->dst_addr[7],
                         (conn->conn_req_parser->dst_addr[8] << 8) | conn->conn_req_parser->dst_addr[9],
                         (conn->conn_req_parser->dst_addr[10] << 8) | conn->conn_req_parser->dst_addr[11],
                         (conn->conn_req_parser->dst_addr[12] << 8) | conn->conn_req_parser->dst_addr[13],
                         (conn->conn_req_parser->dst_addr[14] << 8) | conn->conn_req_parser->dst_addr[15]);
                conn->dst_address[sizeof(conn->dst_address) - 1] = '\0';
            } else if(conn->conn_req_parser->atyp == SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME) {
                strcpy(conn->dst_address, (char *)conn->conn_req_parser->dst_addr);
            } 

            conn->a_type = conn->conn_req_parser->atyp;

            pthread_t thread_id;
            int result = pthread_create(&thread_id, NULL, resolve_host_name, key->data);

            if (result != 0) {
                LOG(ERROR, "Failed to create thread for connecting to host: %s", strerror(result));
                return SOCKS5_ERROR;
            }

            return SOCKS5_CONNECTING;

        } else if (state == CONN_REQ_ERROR) {
            LOG(ERROR, "Invalid request received on fd %d", key->fd);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_CONNECTION_REQ;
}

unsigned int connection_req_write(struct selector_key* key){
        socks5_conn_t* conn = key->data;
        if(conn->is_error_response){
            return SOCKS5_ERROR;
        }else{
            return SOCKS5_COPY;
        }
}

void connection_req_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Connection request phase complete for fd %d", key->fd);
    conn_req_parser_close(conn->conn_req_parser);
    conn->conn_req_parser = NULL;
}