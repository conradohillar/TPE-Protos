#include <handshake.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>

void handshake_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn->handshake_parser = handshake_parser_init();
}

unsigned int handshake_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        
        handshake_state state = handshake_parser_feed(conn->handshake_parser, buffer_read(&conn->in_buff));
        
        if (state == HANDSHAKE_DONE) {
            buffer_write(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, SOCKS5_AUTH_METHOD_NO_AUTH);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);            
            return SOCKS5_AUTH;

        } else if (state == HANDSHAKE_ERROR) {
            buffer_write(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, 0xFF); 
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);            
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_HANDSHAKE;
}

void handshake_on_departure(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = NULL;
}

