#include <auth_parser.h>

auth_parser * auth_parser_init(){return NULL; }
void auth_parser_close(auth_parser *p){}
auth_parser_state auth_parser_feed(auth_parser *p, uint8_t byte){
    return AUTH_PARSER_ERROR;
}