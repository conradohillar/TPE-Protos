#include <conn_req_parser.h>

conn_req_parser *conn_req_parser_create(void) {return NULL;}
void conn_req_parser_close(conn_req_parser *p) {}
conn_req_parser_state conn_req_parser_feed(conn_req_parser *p, uint8_t byte) {return CONN_REQ_PARSER_ERROR;}