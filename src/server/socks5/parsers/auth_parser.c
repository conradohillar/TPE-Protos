#include "../include/auth_parser.h"
#include "../include/parser.h"
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void version(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_VERSION, reading byte: 0x%x", c);
    ret->type = AUTH_VERSION;
    ret->n = 1;
    ret->data[0] = c;
}

static void username_len(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_USERNAME_LEN, reading byte: 0x%x", c);
    ret->type = AUTH_USERNAME_LEN;
    ret->n = 1;
    ret->data[0] = c;
}

static void username(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_USERNAME, reading byte: 0x%x", c);
    ret->type = AUTH_USERNAME;
    ret->n = 1;
    ret->data[0] = c;
}

static void password_len(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_PASSWORD_LEN, reading byte: 0x%x", c);
    ret->type = AUTH_PASSWORD_LEN;
    ret->n = 1;
    ret->data[0] = c;
}

static void password(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_PASSWORD, reading byte: 0x%x", c);
    ret->type = AUTH_PASSWORD;
    ret->n = 1;
    ret->data[0] = c;
}

static void done(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_DONE, reading byte: 0x%x", c);
    ret->type = AUTH_DONE;
    ret->n = 1;
    ret->data[0] = c;
}

static void error(struct parser_event* ret, const uint8_t c) {
    LOG(DEBUG, "STATE: AUTH_ERROR, reading byte: 0x%x", c);
    ret->type = AUTH_ERROR;
    ret->n = 1;
    ret->data[0] = c;
}

static const struct parser_state_transition version_transitions[] = {
    {
        .when = SOCKS5_SUBNEGOTIATION_VERSION,
        .dest = AUTH_USERNAME_LEN,
        .act1 = version,
    },
    {.when = ANY, .dest = AUTH_ERROR, .act1 = error},
};

static const struct parser_state_transition username_len_transitions[] = {
    {.when = 0x00, .dest = AUTH_ERROR, .act1 = error},
    {.when = ANY, .dest = AUTH_USERNAME, .act1 = username_len},
};

static const struct parser_state_transition username_transitions[] = {
    {.when = ANY, .dest = AUTH_USERNAME, .act1 = username},
};

static const struct parser_state_transition password_len_transitions[] = {
    {.when = 0x00, .dest = AUTH_ERROR, .act1 = error},
    {.when = ANY, .dest = AUTH_PASSWORD, .act1 = password_len},
};

static const struct parser_state_transition password_transitions[] = {
    {.when = ANY, .dest = AUTH_PASSWORD, .act1 = password},
};

static const struct parser_state_transition done_transitions[] = {
    {.when = ANY, .dest = AUTH_DONE, .act1 = done},
};

static const struct parser_state_transition error_transitions[] = {
    {.when = ANY, .dest = AUTH_ERROR, .act1 = error},
};

static const struct parser_state_transition* states[] = {
    [AUTH_VERSION] = version_transitions,
    [AUTH_USERNAME_LEN] = username_len_transitions,
    [AUTH_USERNAME] = username_transitions,
    [AUTH_PASSWORD_LEN] = password_len_transitions,
    [AUTH_PASSWORD] = password_transitions,
    [AUTH_DONE] = done_transitions,
    [AUTH_ERROR] = error_transitions,
};

static const size_t states_n[] = {
    [AUTH_VERSION] = N(version_transitions),
    [AUTH_USERNAME_LEN] = N(username_len_transitions),
    [AUTH_USERNAME] = N(username_transitions),
    [AUTH_PASSWORD_LEN] = N(password_len_transitions),
    [AUTH_PASSWORD] = N(password_transitions),
    [AUTH_DONE] = N(done_transitions),
    [AUTH_ERROR] = N(error_transitions),
};

static const struct parser_definition auth_parser_definition = {
    .states = states,
    .states_n = states_n,
    .start_state = AUTH_VERSION,
};

auth_parser* auth_parser_init(void) {
    auth_parser* p = malloc(sizeof(auth_parser));
    if (p == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for auth parser");
        return NULL;
    }
    p->parser = parser_init(parser_no_classes(), &auth_parser_definition);
    p->username_len = 0;
    p->password_len = 0;
    p->username_count = 0;
    p->password_count = 0;
    LOG_MSG(DEBUG, "Auth parser initialized successfully");
    return p;
}

auth_state auth_parser_feed(auth_parser* p, const uint8_t byte) {
    const struct parser_event* e = parser_feed(p->parser, byte);
    switch (e->type) {
    case AUTH_USERNAME_LEN:
        p->username_len = e->data[0];
        break;
    case AUTH_USERNAME:
        p->username[p->username_count++] = e->data[0];
        if (p->username_count >= p->username_len) {
            parser_set(p->parser, AUTH_PASSWORD_LEN);
            p->username[p->username_count] = '\0';
            return AUTH_PASSWORD_LEN;
        }
        break;
    case AUTH_PASSWORD_LEN:
        p->password_len = e->data[0];
        break;
    case AUTH_PASSWORD:
        p->password[p->password_count++] = e->data[0];
        if (p->password_count >= p->password_len) {
            p->password[p->password_count] = '\0';
            parser_set(p->parser, AUTH_DONE);
            return AUTH_DONE;
        }
        break;
    default:
        break;
    }
    return e->type;
}

void auth_parser_close(auth_parser* p) {
    LOG_MSG(DEBUG, "Closing auth parser");
    parser_destroy(p->parser);
    free(p);
}
