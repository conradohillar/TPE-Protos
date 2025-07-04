#include <stdint.h>

#define MAX_USERNAME_LEN 255
#define MAX_PASSWORD_LEN 255

#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#include <auth_table.h>
#include <stdint.h>

typedef enum auth_state {
  AUTH_VERSION,
  AUTH_USERNAME_LEN,
  AUTH_USERNAME,
  AUTH_PASSWORD_LEN,
  AUTH_PASSWORD,
  AUTH_DONE,
  AUTH_ERROR,
} auth_state;

typedef struct auth_parser {
  struct parser *parser;
  uint8_t username_len;
  uint8_t username_count;
  uint8_t username_buf[MAX_USERNAME_LEN];
  uint8_t password_len;
  uint8_t password_count;
  uint8_t password_buf[MAX_PASSWORD_LEN];
} auth_parser;

auth_parser *auth_parser_init(void);

auth_state auth_parser_feed(auth_parser *p, uint8_t byte);

void auth_parser_close(auth_parser *p);

typedef struct {
  auth_state state;
  size_t username_len;
  size_t password_len;
  char username[MAX_USERNAME_LEN + 1];
  char password[MAX_PASSWORD_LEN + 1];
} auth_parser;

auth_parser *auth_parser_init();
void auth_parser_close(auth_parser *p);
auth_state auth_parser_feed(auth_parser *p, uint8_t byte);

#endif // AUTH_PARSER_H