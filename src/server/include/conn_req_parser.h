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
    struct parser* parser;
    uint8_t cmd;            
    uint8_t atyp;           
    uint8_t dst_addr_len;                      
    uint8_t dst_addr[256];  
    uint8_t dst_addr_count; 
    uint16_t dst_port;      
} conn_req_parser;

conn_req_parser* conn_req_parser_init(void);
void conn_req_parser_close(conn_req_parser* p);
conn_req_parser_state conn_req_parser_feed(conn_req_parser* p, uint8_t byte);

#endif
