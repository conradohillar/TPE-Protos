#include <auth.h>

void auth_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Starting authentication for fd %d", key->fd);
    conn->auth_parser = auth_parser_init();
}

// Lee y parsea el mensaje de autenticación username/password
unsigned int auth_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {

        auth_state state = auth_parser_feed(conn->auth_parser, buffer_read(&conn->in_buff));

        if (state == AUTH_DONE) {
            buffer_write(&conn->out_buff, SOCKS5_SUBNEGOTIATION_VERSION);

            if (auth_check_credentials((const char*) conn->auth_parser->username, (const char*) conn->auth_parser->password)) {
                LOG(INFO, "Authentication successful for user: %s on fd %d", conn->auth_parser->username, key->fd);
                buffer_write(&conn->out_buff, SOCKS5_AUTH_OK);
            } else {
                LOG(WARNING, "Authentication failed for user: %s on fd %d", conn->auth_parser->username, key->fd);
                buffer_write(&conn->out_buff, SOCKS5_AUTH_ERROR);
            }

            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_CONNECTION_REQ;

        } else if (state == AUTH_ERROR) {
            LOG(ERROR, "Authentication error for fd %d", key->fd);
            buffer_write(&conn->out_buff, SOCKS5_VERSION);
            buffer_write(&conn->out_buff, AUTH_ERROR);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_AUTH;
}

void auth_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Authentication phase complete for fd %d", key->fd);
    auth_parser_close(conn->auth_parser);
    conn->auth_parser = NULL;
}
