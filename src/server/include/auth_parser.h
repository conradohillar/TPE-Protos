#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#define USERNAME_MAX_LEN 255
#define PASSWORD_MAX_LEN 255

typedef enum auth_parser_state {
  AUTH_PARSER_VERSION,
  AUTH_PARSER_USERNAME,
  AUTH_PARSER_PASSWORD,
  AUTH_PARSER_DONE,
  AUTH_PARSER_ERROR,
} auth_parser_state;

typedef struct auth_parser {
  auth_parser_state state;
  size_t username_len;
  size_t password_len;
  char username[USERNAME_MAX_LEN + 1];
  char password[PASSWORD_MAX_LEN + 1];
} auth_parser;

#endif // AUTH_PARSER_H