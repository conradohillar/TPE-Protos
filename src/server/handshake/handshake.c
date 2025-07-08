#include <handshake.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>

void handshake_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    LOG(DEBUG, "Starting handshake for fd %d", key->fd);
    conn->handshake_parser = handshake_parser_init();
}

unsigned int handshake_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        
        handshake_state state = handshake_parser_feed(conn->handshake_parser, buffer_read(&conn->in_buff));
        
        if (state == HANDSHAKE_DONE) {
            LOG(INFO, "Handshake successful for fd %d", key->fd);
            buffer_write(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, SOCKS5_AUTH_METHOD_USER_PASS);
            selector_set_interest_key(key, OP_WRITE);
                       
            return SOCKS5_AUTH;

        } else if (state == HANDSHAKE_ERROR) {
            LOG(ERROR, "Handshake error for fd %d", key->fd);
            buffer_write(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, 0xFF); 
            selector_set_interest_key(key, OP_WRITE);
                   
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_HANDSHAKE;
}

void handshake_on_departure(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    LOG(DEBUG, "Handshake phase complete for fd %d", key->fd);
    handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = NULL;
}

