#include <socks5_stm.h>

struct state_definition socks5_states[] = {
    {
        .state = SOCKS5_HANDSHAKE_READ,
        .on_arrival = socks5_on_arrival,
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
        .on_arrival = socks5_on_arrival,
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
        .on_arrival = socks5_on_arrival,
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
        .on_arrival = socks5_on_arrival,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_ERROR,
        .on_arrival = socks5_on_arrival,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    }};

struct state_machine socks5_stm = {
    .states = socks5_states,
    .max_state = sizeof(socks5_states) / sizeof(socks5_states[0]),
};

void socks5_on_arrival(unsigned int state, struct selector_key *key) {
    switch(state) {
        case SOCKS5_HANDSHAKE_READ:
            handshake_parser_init(key);
            break;
        case SOCKS5_AUTH_WRITE:
            auth_parser_init(key);
            break;
        case SOCKS5_CONNECTION_REQ_READ:
            connection_req_init(key);
            break;
        case SOCKS5_DONE:
            handle_done(key);
            break;
        case SOCKS5_ERROR:
            handle_error(key);
            break;
        default:
            // Otros estados no requieren inicialización especial
            break;
    }
}