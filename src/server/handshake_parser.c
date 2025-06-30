#include "handshake_parser.h"

static void handle_version(struct parser_event *ret, const uint8_t c) {
  ret->type = HANDSHAKE_VERSION;
  ret->n = 1;
  ret->data[0] = c;
}

static void handle_n_methods(struct parser_event *ret, const uint8_t c) {
  ret->type = HANDSHAKE_N_METHODS;
  ret->n = 1;
  ret->data[0] = c;
}

static void handle_version_error(struct parser_event *ret, const uint8_t c) {
  ret->type = HANDSHAKE_ERROR;
  ret->n = 1;
  ret->data[0] = c;
  ret->next =
}

static void do_nothing(struct parser_event *ret, const uint8_t c) { return; }

static const struct parser_state_transition version_transitions[] = {
    {.when = SOCKS5_VERSION, .dest = HANDSHAKE_N_METHODS, .act1 = do_nothing},
    {.when = ANY, .dest = HANDSHAKE_ERROR, .act1 = handle_version_error},
};

static const struct parser_state_transition n_methods_transitions[] = {
    {.when = SOCKS5_AUTH_METHOD_USER_PASS,
     .dest = HANDSHAKE_DONE,
     .act1 = do_nothing},
    {.when = ANY, .dest = HANDSHAKE_ERROR, .act1 = handle_n_methods},
};
