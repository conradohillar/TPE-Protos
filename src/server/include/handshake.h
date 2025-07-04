#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include "parser.h"
#include <stddef.h>
#include <stdint.h>

#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_METHOD_NO_AUTH 0x00
#define SOCKS5_AUTH_METHOD_GSSAPI 0x01
#define SOCKS5_AUTH_METHOD_USER_PASS 0x02
#define SOCKS5_AUTH_METHOD_NO_ACCEPTABLE 0xFF

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

handshake_parser *handshake_parser_init(void);

handshake_state handshake_parser_feed(handshake_parser *p, uint8_t byte);

void handshake_parser_close(handshake_parser *p);

#endif