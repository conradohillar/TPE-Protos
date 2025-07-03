#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#include <auth_table.h>
#include <stdint.h>

typedef enum auth_parser_state {
  AUTH_PARSER_VERSION,
  AUTH_PARSER_USERNAME,
  AUTH_PARSER_PASSWORD,
  AUTH_PARSER_DONE,
  AUTH_PARSER_ERROR,
} auth_parser_state;

typedef struct {
  auth_parser_state state;
  size_t username_len;
  size_t password_len;
  char username[MAX_USERNAME_LEN + 1];
  char password[MAX_PASSWORD_LEN + 1];
} auth_parser;

auth_parser * auth_parser_init();
void auth_parser_close(auth_parser *p);
auth_parser_state auth_parser_feed(auth_parser *p, uint8_t byte);

#endif // AUTH_PARSER_H