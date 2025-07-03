#include <conn_req.h>
#include <socks5.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>

// Inicializa el buffer y offsets para la etapa de request
void connection_req_init(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    if(conn->conn_req_parser) conn_req_parser_close(conn->conn_req_parser);
    conn->conn_req_parser = conn_req_parser_create();
    conn->len = 0;
    conn->parsed = 0;
    conn->reply_len = 0;
    conn->reply_sent = 0;
}

// Lee y parsea el mensaje de request SOCKS5
unsigned int connection_req_read(struct selector_key *key) {
    socks5_conn_t *conn = ATTACHMENT(key);
    ssize_t n = read(conn->client_fd, conn->buf + conn->len, SOCKS5_BUFF_MAX_LEN - conn->len);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SOCKS5_CONNECTION_REQ_READ;
        } else {
            return SOCKS5_ERROR;
        }
    }
    if (n == 0) {
        return SOCKS5_ERROR;
    }
    conn->len += n;
    while (conn->parsed < conn->len) {
        conn_req_parser_state state = conn_req_parser_feed(conn->conn_req_parser, conn->buf[conn->parsed]);
        conn->parsed++;
        if (state == CONN_REQ_PARSER_DONE) {
            // RESOLVER LA SOLICITUD
            conn->reply[0] = 0x05; // VER
            conn->reply[1] = 0x00; // REP: succeeded
            conn->reply[2] = 0x00; // RSV
            conn->reply[3] = 0x01; // ATYP IPv4
            memset(conn->reply+4, 0, 6); // BND.ADDR + BND.PORT (0.0.0.0:0)
            conn->reply_len = 10;
            return SOCKS5_CONNECTION_REQ_WRITE;
        } else if (state == CONN_REQ_PARSER_ERROR) {
            conn->reply[0] = 0x05;
            conn->reply[1] = 0x01; // REP: general failure
            conn->reply[2] = 0x00;
            conn->reply[3] = 0x01;
            memset(conn->reply+4, 0, 6);
            conn->reply_len = 10;
            return SOCKS5_CONNECTION_REQ_WRITE;
        }
    }
    return SOCKS5_CONNECTION_REQ_READ;
}