#ifndef CONN_REQ_PARSER_H
#define CONN_REQ_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef enum conn_req_parser_state {
    CONN_REQ_PARSER_VERSION,
    CONN_REQ_PARSER_CMD,
    CONN_REQ_PARSER_RSV,
    CONN_REQ_PARSER_ATYP,
    CONN_REQ_PARSER_DST_ADDR,
    CONN_REQ_PARSER_DST_PORT,
    CONN_REQ_PARSER_DONE,
    CONN_REQ_PARSER_ERROR,
} conn_req_parser_state;

typedef struct {
    uint8_t ver;      // Versión del protocolo (0x05)
    uint8_t cmd;      // Comando (0x01 para CONNECT)
    uint8_t rsv;      // Reservado, debe ser 0x00
    uint8_t atyp;     // Tipo de dirección (0x01 IPv4, 0x03 DOMAIN, 0x04 IPv6)
    char dst_addr[256]; // Dirección destino (IPv4/IPv6 o dominio)
    uint16_t dst_port; // Puerto destino
} conn_req_parser;

conn_req_parser *conn_req_parser_create(void) {return NULL;}
void conn_req_parser_close(conn_req_parser *p) {}
conn_req_parser_state conn_req_parser_feed(conn_req_parser *p, uint8_t byte) {return CONN_REQ_PARSER_ERROR;}

#endif