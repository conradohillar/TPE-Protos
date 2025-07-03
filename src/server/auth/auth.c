#include <auth.h>

void auth_parser_init(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    if (conn->auth_parser) auth_parser_close(conn->auth_parser);
    conn->auth_parser = auth_parser_create();
    conn->len = 0;
    conn->parsed = 0;
    conn->reply_len = 0;
    conn->reply_sent = 0;
}

// Lee y parsea el mensaje de autenticación username/password
unsigned int auth_read(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    ssize_t n = read(conn->client_fd, conn->buf + conn->len, SOCKS5_BUFF_MAX_LEN - conn->len);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SOCKS5_AUTH_READ; // No hay datos, esperar próximo evento
        } else {
            return SOCKS5_ERROR; // Error real
        }
    }
    if (n == 0) {
        return SOCKS5_ERROR; // EOF: el cliente cerró la conexión
    }
    conn->len += n;
    // Parsear byte a byte
    while (conn->parsed < conn->len) {
        auth_parser_state state = auth_parser_feed(conn->auth_parser, conn->buf[conn->parsed]);
        conn->parsed++;
        if (state == AUTH_PARSER_DONE) {
            conn->reply_len = 2;
            conn->reply_sent = 0;
            // Validar credenciales
            if (auth_check_credentials(conn->auth_parser->username, conn->auth_parser->password)) {
                conn->reply[0] = 0x01; // VER
                conn->reply[1] = 0x00; // STATUS OK
            } else {
                conn->reply[0] = 0x01;
                conn->reply[1] = 0x01; // STATUS FAIL
            }
            return SOCKS5_AUTH_WRITE;

        } else if (state == AUTH_PARSER_ERROR) {
            conn->reply[0] = 0x01;
            conn->reply[1] = 0x01;
            conn->reply_len = 2;
            conn->reply_sent = 0;
            return SOCKS5_ERROR; 
        }
    }
    return SOCKS5_AUTH_READ; // Esperar más datos
}
