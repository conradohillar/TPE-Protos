#include "../include/handshake.h"

void handshake_parser_init(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    if (conn->handshake_parser) handshake_parser_close(conn->handshake_parser);
    conn->handshake_parser = handshake_parser_create();
    conn->len = 0;
    conn->parsed = 0;
    conn->status = 0;
    conn->reply_len = 0;
    conn->reply_sent = 0;
}

void handshake_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    ssize_t n = read(conn->client_fd, conn->buf + conn->len, sizeof(conn->buf) - conn->len);
    if (n <= 0) {
        // Error o desconexión
        // Pasar a estado de error
        // stm_next_state(key->s, SOCKS5_ERROR);
        return;
    }
    conn->len += n;
    // Parsear byte a byte
    while (conn->parsed < conn->len && conn->status == 0) {
        handshake_state state = handshake_parser_feed(conn->handshake_parser, conn->buf[conn->parsed]);
        conn->parsed++;
        if (state == HANDSHAKE_DONE) {
            conn->reply[0] = SOCKS5_VERSION;
            conn->reply[1] = SOCKS5_AUTH_METHOD_USER_PASS;
            conn->reply_len = 2;
            conn->status = 1;
            // stm_next_state(key->s, SOCKS5_HANDSHAKE_WRITE);
            break;
        } else if (state == HANDSHAKE_ERROR) {
            conn->reply[0] = SOCKS5_VERSION;
            conn->reply[1] = 0xFF;
            conn->reply_len = 2;
            conn->status = -1;
            // stm_next_state(key->s, SOCKS5_HANDSHAKE_WRITE);
            break;
        }
    }
    // Si status != 0, pasar a estado de escritura
}

void handshake_write(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (conn->reply_sent < conn->reply_len) {
        ssize_t n = write(conn->client_fd, conn->reply + conn->reply_sent, conn->reply_len - conn->reply_sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            // Error fatal
            // stm_next_state(key->s, SOCKS5_ERROR);
            return;
        }
        conn->reply_sent += n;
    }
    if (conn->reply_sent == conn->reply_len) {
        if (conn->status == 1) {
            // stm_next_state(key->s, SOCKS5_AUTH_READ);
        } else {
            // stm_next_state(key->s, SOCKS5_ERROR);
        }
    }
}

