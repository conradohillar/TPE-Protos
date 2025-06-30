#include "socks5.h"
#include <stddef.h>

struct state_definition socks5_states[] = {
    {
        .state = SOCKS5_HANDSHAKE_READ,
        .on_arrival = handshake_parser_init,
        .on_departure = NULL,
        .on_read_ready = handshake_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_HANDSHAKE_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = handshake_write,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_AUTH_READ,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = auth_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_AUTH_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = auth_write,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_CONNECTION_REQ_READ,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = connection_req_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_CONNECTION_REQ_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = connection_req_write,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_DONE,
        .on_arrival = handle_done,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_ERROR,
        .on_arrival = handle_error,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    }};

struct state_machine socks5_stm = {
    .states = socks5_states,
    .num_states = sizeof(socks5_states) / sizeof(socks5_states[0]),
};
