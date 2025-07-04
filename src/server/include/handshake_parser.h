#ifndef HANDSHAKE_PARSER_H
#define HANDSHAKE_PARSER_H

#include <parser.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


typedef enum handshake_state {
  HANDSHAKE_VERSION,
  HANDSHAKE_NMETHODS,
  HANDSHAKE_METHODS,
  HANDSHAKE_DONE,
  HANDSHAKE_ERROR
} handshake_state;

typedef struct handshake_parser {
  struct parser *parser;
  uint8_t nmethods;
  uint8_t method_count;
} handshake_parser;

handshake_parser *handshake_parser_init();

handshake_state handshake_parser_feed(handshake_parser *p, uint8_t byte);

void handshake_parser_close(handshake_parser *p);

int handshake_process(handshake_parser *p);

#endif