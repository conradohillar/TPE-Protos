#include <conn_req.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>

// Inicializa el buffer y offsets para la etapa de request
void connection_req_on_arrival(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn->conn_req_parser = conn_req_parser_create();
}

// Lee y parsea el mensaje de request SOCKS5
unsigned int connection_req_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        size_t avail;
        uint8_t *read_ptr = buffer_read_ptr(&conn->in_buff, &avail);
        conn_req_parser_state state = conn_req_parser_feed(conn->conn_req_parser, *read_ptr);
        buffer_read_adv(&conn->in_buff, 1);
        if (state == CONN_REQ_PARSER_DONE) {
            // RESOLVER LA SOLICITUD
            conn->out_buff.data[0] = 0x05; // VER
            conn->out_buff.data[1] = 0x00; // REP: succeeded
            conn->out_buff.data[2] = 0x00; // RSV
            conn->out_buff.data[3] = 0x01; // ATYP IPv4
            memset(conn->out_buff.data+4, 0, 6); // BND.ADDR + BND.PORT (0.0.0.0:0)
            buffer_write_adv(&conn->out_buff, 10);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_DONE;

        } else if (state == CONN_REQ_PARSER_ERROR) {
            conn->out_buff.data[0] = 0x05;
            conn->out_buff.data[1] = 0x01; // REP: general failure
            conn->out_buff.data[2] = 0x00;
            conn->out_buff.data[3] = 0x01;
            memset(conn->out_buff.data+4, 0, 6);
            buffer_write_adv(&conn->out_buff, 10);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_CONNECTION_REQ;
}

void connection_req_on_departure(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    conn_req_parser_close(conn->conn_req_parser);
    conn->conn_req_parser = NULL;
}