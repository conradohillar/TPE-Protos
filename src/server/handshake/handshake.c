#include <handshake.h>

void handshake_parser_init(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    if (conn->handshake_parser) handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = handshake_parser_create();
    conn->len = 0;
    conn->parsed = 0;
    conn->reply_len = 0;
    conn->reply_sent = 0;
}

unsigned int handshake_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    // Parsear byte a byte
    while (conn->parsed < conn->len) {
        handshake_state state = handshake_parser_feed(conn->handshake_parser, conn->buf[conn->parsed]);
        conn->parsed++;
        if (state == HANDSHAKE_DONE) {
            conn->reply[0] = SOCKS5_VERSION;
            conn->reply[1] = SOCKS5_AUTH_METHOD_USER_PASS;
            conn->reply_len = 2;
            return SOCKS5_HANDSHAKE_WRITE;

        } else if (state == HANDSHAKE_ERROR) {
            conn->reply[0] = SOCKS5_VERSION;
            conn->reply[1] = 0xFF;
            conn->reply_len = 2;
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_HANDSHAKE_READ; // Esperar más datos
}

