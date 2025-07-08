#include "../include/handshake_parser.h"
#include "../include/handshake.h"
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>

static void version(struct parser_event *ret, const uint8_t c) {
  LOG(DEBUG, "STATE: HANDSHAKE_VERSION, reading byte: 0x%x", c);
  ret->type = HANDSHAKE_VERSION;
  ret->n = 1;
  ret->data[0] = c;
}

static void nmethods(struct parser_event *ret, const uint8_t c) {
  LOG(DEBUG, "STATE: HANDSHAKE_NMETHODS, reading byte: 0x%x", c);
  ret->type = HANDSHAKE_NMETHODS;
  ret->n = 1;
  ret->data[0] = c;
}

static void methods(struct parser_event *ret, const uint8_t c) {
  LOG(DEBUG, "STATE: HANDSHAKE_METHODS, reading byte: 0x%x", c);
  ret->type = HANDSHAKE_METHODS;
  ret->n = 1;
  ret->data[0] = c;
}

static void done(struct parser_event *ret, const uint8_t c) {
  LOG(DEBUG, "STATE: HANDSHAKE_DONE, reading byte: 0x%x", c);
  ret->type = HANDSHAKE_DONE;
  ret->n = 1;
  ret->data[0] = c;
}

static void error(struct parser_event *ret, const uint8_t c) {
  LOG(DEBUG, "STATE: HANDSHAKE_ERROR, reading byte: 0x%x", c);
  ret->type = HANDSHAKE_ERROR;
  ret->n = 1;
  ret->data[0] = c;
}

static const struct parser_state_transition version_transitions[] = {
    {.when = SOCKS5_VERSION, .dest = HANDSHAKE_NMETHODS, .act1 = version},
    {.when = ANY, .dest = HANDSHAKE_ERROR, .act1 = error},
};

static const struct parser_state_transition nmethods_transitions[] = {
    {.when = ANY, .dest = HANDSHAKE_METHODS, .act1 = nmethods},
};

static const struct parser_state_transition methods_transitions[] = {
    {.when = SOCKS5_AUTH_METHOD_USER_PASS,
     .dest = HANDSHAKE_DONE,
     .act1 = done},
    {.when = ANY, .dest = HANDSHAKE_METHODS, .act1 = methods},
};

static const struct parser_state_transition error_transitions[] = {
    {.when = ANY, .dest = HANDSHAKE_ERROR, .act1 = error},
};

static const struct parser_state_transition done_transitions[] = {
    {.when = ANY, .dest = HANDSHAKE_DONE, .act1 = done},
};

static const struct parser_state_transition *states[] = {
    [HANDSHAKE_VERSION] = version_transitions,
    [HANDSHAKE_NMETHODS] = nmethods_transitions,
    [HANDSHAKE_METHODS] = methods_transitions,
    [HANDSHAKE_ERROR] = error_transitions,
    [HANDSHAKE_DONE] = done_transitions,
};

static const size_t states_n[] = {
    [HANDSHAKE_VERSION] = N(version_transitions),
    [HANDSHAKE_NMETHODS] = N(nmethods_transitions),
    [HANDSHAKE_METHODS] = N(methods_transitions),
    [HANDSHAKE_ERROR] = N(error_transitions),
    [HANDSHAKE_DONE] = N(done_transitions),
};

static const struct parser_definition handshake_parser_definition = {
    .states = states,
    .states_n = states_n,
    .start_state = HANDSHAKE_VERSION,
};

handshake_parser *handshake_parser_init() {
  handshake_parser *p = malloc(sizeof(handshake_parser));
  if (p == NULL) {
    LOG_MSG(ERROR, "Failed to allocate memory for handshake parser");
    return NULL;
  }

  p->parser = parser_init(parser_no_classes(), &handshake_parser_definition);
  p->nmethods = 0;
  p->method_count = 0;
  return p;
}

handshake_state handshake_parser_feed(handshake_parser *p, uint8_t byte) {
  const struct parser_event *e = parser_feed(p->parser, byte);

  switch (e->type) {
  case HANDSHAKE_NMETHODS:
    p->nmethods = e->data[0];
    break;
  case HANDSHAKE_METHODS:
    p->method_count++;
    if (p->method_count > p->nmethods ||
        (p->method_count == p->nmethods &&
         e->data[0] != SOCKS5_AUTH_METHOD_USER_PASS)) {
      // si se leyeron todos los métodos y el parser no "encontró" el metodo
      // SOCKS5_AUTH_METHOD_USER_PASS, entonces se retorna un error
      return HANDSHAKE_ERROR;
    }

    break;
  default:
    break;
  }
  return e->type;
}

void handshake_parser_close(handshake_parser *p) { parser_destroy(p->parser); }
