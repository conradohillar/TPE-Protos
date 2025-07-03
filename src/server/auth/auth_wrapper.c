#include "../include/auth_wrapper.h"

// Lee y parsea el mensaje de autenticación username/password
void auth_read(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    ssize_t n = read(conn->client_fd, conn->buf + conn->len, SOCKS5_BUFF_MAX_LEN - conn->len);
    if (n <= 0) {
        // Error o desconexión
        conn->status = -1;
        return;
    }
    conn->len += n;
    // Parsear byte a byte
    while (conn->parsed < conn->len && conn->status == 0) {
        auth_parser_state state = auth_parser_feed(conn->auth_parser, conn->buf[conn->parsed]);
        conn->parsed++;
        if (state == AUTH_PARSER_DONE) {
            // Validar credenciales
            if (auth_validate(conn->auth_parser->username, conn->auth_parser->password)) {
                conn->reply[0] = 0x01; // VER
                conn->reply[1] = 0x00; // STATUS OK
                conn->status = 1;
            } else {
                conn->reply[0] = 0x01;
                conn->reply[1] = 0x01; // STATUS FAIL
                conn->status = -1;
            }
            conn->reply_len = 2;
            conn->reply_sent = 0;
            break;
        } else if (state == AUTH_PARSER_ERROR) {
            conn->reply[0] = 0x01;
            conn->reply[1] = 0x01;
            conn->reply_len = 2;
            conn->reply_sent = 0;
            conn->status = -1;
            break;
        }
    }
}

// Escribe la respuesta de autenticación al cliente
void auth_write(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    while (conn->reply_sent < conn->reply_len) {
        ssize_t n = write(conn->client_fd, conn->reply + conn->reply_sent, conn->reply_len - conn->reply_sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            conn->status = -1;
            return;
        }
        conn->reply_sent += n;
    }
    // Si se envió toda la respuesta, avanzar de estado según éxito o error
    // (esto lo maneja la máquina de estados principal)
}
