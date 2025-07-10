#include <handshake.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>
#include <socks5_responses.h>

void handshake_on_arrival(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Starting handshake for fd %d", key->fd);
    conn->handshake_parser = handshake_parser_init();
}

unsigned int handshake_read(struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {

        handshake_state state = handshake_parser_feed(conn->handshake_parser, buffer_read(&conn->in_buff));

        if (state == HANDSHAKE_DONE) {
            LOG(INFO, "Handshake successful for fd %d", key->fd);
            socks5_handshake_response response = create_handshake_response(conn->handshake_parser->no_auth, conn->handshake_parser->user_pass_auth);    
            buffer_write_struct(&conn->out_buff, &response, HANDSHAKE_RESPONSE_SIZE);
            selector_set_interest_key(key, OP_WRITE);
            conn->is_error_response = !(conn->handshake_parser->no_auth || conn->handshake_parser->user_pass_auth);
        } else if (state == HANDSHAKE_ERROR) {
            LOG(ERROR, "Handshake error for fd %d", key->fd);
            return SOCKS5_ERROR;            
        }
    }
    return SOCKS5_HANDSHAKE;
}

unsigned int handshake_write(struct selector_key* key){
        socks5_conn_t* conn = key->data;
        if(conn->is_error_response){
            return SOCKS5_ERROR;
        }else{
            if(conn->handshake_parser->user_pass_auth){
                return SOCKS5_AUTH;
            }
            return SOCKS5_CONNECTION_REQ;
        }
}

void handshake_on_departure(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    LOG(DEBUG, "Handshake phase complete for fd %d", key->fd);
    handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = NULL;
}
