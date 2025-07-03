#include <auth.h>

void auth_on_arrival(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn->auth_parser = auth_parser_init();
}

// Lee y parsea el mensaje de autenticación username/password
unsigned int auth_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        size_t avail;
        uint8_t *read_ptr = buffer_read_ptr(&conn->in_buff, &avail);
        auth_parser_state state = auth_parser_feed(conn->auth_parser, *read_ptr);
        buffer_read_adv(&conn->in_buff, 1);
        if (state == AUTH_PARSER_DONE) {
            conn->out_buff.data[0] = 0x01; // VER
            if (auth_check_credentials(conn->auth_parser->username, conn->auth_parser->password)) {
                conn->out_buff.data[1] = 0x00; // STATUS OK
            } else {
                conn->out_buff.data[1] = 0x01; // STATUS FAIL
            }
            buffer_write_adv(&conn->out_buff, 2);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_CONNECTION_REQ;

        } else if (state == AUTH_PARSER_ERROR) {
            conn->out_buff.data[0] = 0x01;
            conn->out_buff.data[1] = 0x01;
            buffer_write_adv(&conn->out_buff, 2);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_AUTH;
}

void auth_on_departure(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    auth_parser_close(conn->auth_parser);
    conn->auth_parser = NULL;
}
