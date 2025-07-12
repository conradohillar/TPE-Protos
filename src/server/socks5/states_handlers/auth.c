#include <auth.h>

void auth_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Starting authentication for fd %d", key->fd);
    conn->auth_parser = auth_parser_init();
}

unsigned int auth_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {

        auth_state state = auth_parser_feed(conn->auth_parser, buffer_read(&conn->in_buff));

        if (state == AUTH_DONE) {
            bool auth_ok = auth_check_credentials((const char*) conn->auth_parser->username, (const char*) conn->auth_parser->password);
            socks5_auth_response response = create_auth_response(auth_ok);
            buffer_write_struct(&conn->out_buff, &response, AUTH_RESPONSE_SIZE);
            buffer_reset(&conn->in_buff);
            conn->is_error_response = !auth_ok;
            selector_set_interest_key(key, OP_WRITE);
            if(auth_ok){
                strcpy(conn->username, (const char*) conn->auth_parser->username);
            }
        } else if (state == AUTH_ERROR) {
            LOG(ERROR, "Failed to parse auth packet for fd: %d", key->fd);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_AUTH;
}

unsigned int auth_write(struct selector_key* key){
        socks5_conn_t* conn = key->data;
        if(conn->is_error_response){
            return SOCKS5_ERROR;
        }else{
            return SOCKS5_CONNECTION_REQ;
        }
}

void auth_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Authentication phase complete for fd %d", key->fd);
    auth_parser_close(conn->auth_parser);
    conn->auth_parser = NULL;
}
