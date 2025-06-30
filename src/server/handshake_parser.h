#ifndef HANDSHAKE_PARSER_H
#define HANDSHAKE_PARSER_H

#include "parser.h"
#include <stdint.h>

#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_METHOD_USER_PASS 0x02

typedef enum handshake_parser_state {
  HANDSHAKE_VERSION,
  HANDSHAKE_N_METHODS,
  HANDSHAKE_METHODS,
  HANDSHAKE_DONE,
  HANDSHAKE_ERROR,
} handshake_parser_state;

typedef enum handshake_parser_event {
  HANDSHAKE_READING_VERSION,
  HANDSHAKE_READING_N_METHODS,
  HANDSHAKE_READING_METHODS,
  HANDSHAKE_COMPLETED,
  HANDSHAKE_ERROR,
} handshake_parser_event;

#endif
