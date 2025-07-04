#include <auth.h>

void auth_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn->auth_parser = auth_parser_init();
}

// Lee y parsea el mensaje de autenticación username/password
unsigned int auth_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
       
        auth_parser_state state = auth_parser_feed(conn->auth_parser, buffer_read(&conn->in_buff));

        if (state == AUTH_PARSER_DONE) {
            buffer_write(&conn->out_buff, SOCKS5_VERSION);

            if (auth_check_credentials(conn->auth_parser->username, conn->auth_parser->password)) {
                 buffer_write(&conn->out_buff, AUTH_OK);
            } else {
                buffer_write(&conn->out_buff, AUTH_ERROR);
            }

            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_CONNECTION_REQ;

        } else if (state == AUTH_PARSER_ERROR) {
            buffer_wirte(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, AUTH_ERROR);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_AUTH;
}

void auth_on_departure(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    auth_parser_close(conn->auth_parser);
    conn->auth_parser = NULL;
}
