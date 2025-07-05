#include <socks5_stm.h>
#include <handshake.h>
#include <auth.h>
#include <conn_req.h>
#include <stm.h>

// Preparar la conexión para relay de datos (cliente <-> destino)
// O limpiar recursos si corresponde
static void handle_done(unsigned state, struct selector_key *key);
static void handle_error(unsigned state, struct selector_key *key);


struct state_definition socks5_states[] = {
    {
        .state = SOCKS5_HANDSHAKE,
        .on_arrival = handshake_on_arrival,
        .on_departure = handshake_on_departure,
        .on_read_ready = handshake_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_AUTH,
        .on_arrival = auth_on_arrival,
        .on_departure = auth_on_departure,
        .on_read_ready = auth_read,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = SOCKS5_CONNECTION_REQ,
        .on_arrival = connection_req_on_arrival,
        .on_departure = connection_req_on_departure,
        .on_read_ready = connection_req_read,
        .on_write_ready = NULL,
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


struct state_machine *socks5_stm_init() {
    struct state_machine *stm = malloc(sizeof(struct state_machine));
    if (stm == NULL) {
        perror("malloc");
        return NULL;
    }
    stm->states = socks5_states;
    stm->max_state = sizeof(socks5_states) / sizeof(socks5_states[0]);
    stm->initial = SOCKS5_HANDSHAKE;
    stm->current = NULL;
    return stm;
}

// --- Estados finales ---
static void handle_done(unsigned state, struct selector_key *key) {
    return;
}

static void handle_error(unsigned state, struct selector_key *key) {
    return;
}