typedef enum auth_parser_state {
  AUTH_PARSER_VERSION,
  AUTH_PARSER_USERNAME,
  AUTH_PARSER_PASSWORD,
  AUTH_PARSER_DONE,
  AUTH_PARSER_ERROR,
} auth_parser_state;
