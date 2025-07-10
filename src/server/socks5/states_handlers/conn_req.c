#include <conn_req.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>
#include <conn_req_parser.h>
#include <netutils.h>
#include <pthread.h>

void connection_req_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Starting connection request phase for fd %d", key->fd);
    conn->conn_req_parser = conn_req_parser_init();
}

unsigned int connection_req_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {

        conn_req_parser_state state = conn_req_parser_feed(conn->conn_req_parser, buffer_read(&conn->in_buff));

        LOG(INFO, "Processing connection request for fd %d, state: %d", key->fd, state);
        if (state == CONN_REQ_DONE) {
            LOG(INFO, "Connection request received for fd %d, connecting to destination", key->fd);
            LOG(INFO, "Destination address: %s, port: %d", conn->conn_req_parser->dst_addr, conn->conn_req_parser->dst_port);

            selector_set_interest_key(key, OP_NOOP);

            conn->dst_port = conn->conn_req_parser->dst_port;
            strcpy(conn->dst_address, (char*) conn->conn_req_parser->dst_addr);

            pthread_t thread_id;
            int result = pthread_create(&thread_id, NULL, resolve_host_name, key->data);

            if (result != 0) {
                LOG(ERROR, "Failed to create thread for connecting to host: %s", strerror(result));
            }

            return SOCKS5_CONNECTING;

        } else if (state == CONN_REQ_ERROR) {
            LOG(ERROR, "Connection request error for fd %d", key->fd);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_CONNECTION_REQ;
}

void connection_req_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Connection request phase complete for fd %d", key->fd);
    conn_req_parser_close(conn->conn_req_parser);
    conn->conn_req_parser = NULL;
}