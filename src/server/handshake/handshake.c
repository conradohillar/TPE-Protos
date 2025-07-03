#include <handshake.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>

void handshake_on_arrival(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn->handshake_parser = handshake_parser_init();
}

unsigned int handshake_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        size_t avail;
        uint8_t *read_ptr = buffer_read_ptr(&conn->in_buff, &avail);
        handshake_state state = handshake_parser_feed(conn->handshake_parser, *read_ptr);
        buffer_read_adv(&conn->in_buff, 1);

        if (state == HANDSHAKE_DONE) {
            conn->out_buff.data[0] = SOCKS5_VERSION;
            conn->out_buff.data[1] = SOCKS5_AUTH_METHOD_USER_PASS;
            buffer_write_adv(&conn->out_buff, 2);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);            
            return SOCKS5_AUTH;

        } else if (state == HANDSHAKE_ERROR) {
            conn->out_buff.data[0] = SOCKS5_VERSION;
            conn->out_buff.data[1] = 0xFF;
            buffer_write_adv(&conn->out_buff, 2);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);            
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_HANDSHAKE;
}

void handshake_on_departure(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = NULL;
}

