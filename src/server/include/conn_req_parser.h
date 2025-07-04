#ifndef CONN_REQ_PARSER_H
#define CONN_REQ_PARSER_H

#include "parser.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef enum conn_req_parser_state {
  CONN_REQ_VERSION,
  CONN_REQ_CMD,
  CONN_REQ_RSV,
  CONN_REQ_ATYP,
  CONN_REQ_DOM_LEN,
  CONN_REQ_DST_ADDR,
  CONN_REQ_DST_PORT_BYTE1,
  CONN_REQ_DST_PORT_BYTE2,
  CONN_REQ_DONE,
  CONN_REQ_ERROR,
} conn_req_parser_state;

typedef struct {
  struct parser *parser;
  uint8_t cmd;  // Comando (0x01 para CONNECT)
  uint8_t atyp; // Tipo de dirección (0x01 IPv4, 0x03 DOMAIN, 0x04 IPv6)
  uint8_t dst_addr_len;   // Longitud del dominio destino (en caso de que sea un
                          // dominio)
  uint8_t dst_addr[256];  // Dirección destino (IPv4/IPv6 o dominio)
  uint8_t dst_addr_count; // Contador de bytes leídos de dst_addr
  uint16_t dst_port;      // Puerto destino
} conn_req_parser;

conn_req_parser *conn_req_parser_init(void);
void conn_req_parser_close(conn_req_parser *p);
conn_req_parser_state conn_req_parser_feed(conn_req_parser *p, uint8_t byte);

#endif