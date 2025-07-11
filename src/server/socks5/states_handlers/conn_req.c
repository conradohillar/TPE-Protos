#include <conn_req.h>

void set_dst_address(char* dst_address, const uint8_t* addr, size_t addr_len, uint8_t a_type);

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
            conn->a_type = conn->conn_req_parser->atyp;
            set_dst_address(conn->dst_address, conn->conn_req_parser->dst_addr, conn->conn_req_parser->dst_addr_len, conn->a_type);

            if(conn->a_type == SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME){
                pthread_t thread_id;
                int result = pthread_create(&thread_id, NULL, resolve_host_name, key->data);

                if (result != 0) {
                    LOG(ERROR, "Failed to create thread for connecting to host: %s", strerror(result));
                    return SOCKS5_ERROR;
                }

                return SOCKS5_CONNECTING;
            }else{
                resolve_host_name(key->data);
                return SOCKS5_CONNECTING;
            }
            

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

void set_dst_address(char* dst_address, const uint8_t* addr, size_t addr_len, uint8_t a_type) {
    if (a_type == SOCKS5_CONN_REQ_ATYP_IPV4) {
        snprintf(dst_address, DOMAIN_NAME_MAX_LENGTH, "%d.%d.%d.%d",
                 addr[0], addr[1], addr[2], addr[3]);
    } else if (a_type == SOCKS5_CONN_REQ_ATYP_IPV6) {
        if (inet_ntop(AF_INET6, addr, dst_address, DOMAIN_NAME_MAX_LENGTH) == NULL) {
            strncpy(dst_address, "<invalid IPv6>", DOMAIN_NAME_MAX_LENGTH);
        }
    } else if (a_type == SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME) {
        size_t max_copy = addr_len < DOMAIN_NAME_MAX_LENGTH - 1 ? addr_len : DOMAIN_NAME_MAX_LENGTH - 1;
        memcpy(dst_address, addr, max_copy);
        dst_address[max_copy] = '\0';
    }
}
