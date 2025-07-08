#include <socks5_stm.h>
#include <handshake.h>
#include <auth.h>
#include <conn_req.h>
#include <stm.h>
#include <socks5_copy.h>
#include <logger.h>
#include <unistd.h>

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
    },
    {
        .state = SOCKS5_AUTH,
        .on_arrival = auth_on_arrival,
        .on_departure = auth_on_departure,
        .on_read_ready = auth_read
    },
    {
        .state = SOCKS5_CONNECTION_REQ,
        .on_arrival = connection_req_on_arrival,
        .on_departure = connection_req_on_departure,
        .on_read_ready = connection_req_read
    },
    {
        .state = SOCKS5_COPY,
        .on_arrival     = copy_on_arrival,
        .on_read_ready  = copy_read         // Aca leemos del buffer interno y reenviamos al destino
    }, 
    {
        .state = SOCKS5_DONE,
        .on_arrival = handle_done
    },
    {
        .state = SOCKS5_ERROR,
        .on_arrival = handle_error
    }};


struct state_machine *socks5_stm_init() {
    struct state_machine *stm = malloc(sizeof(struct state_machine));
    if (stm == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for SOCKS5 state machine");
        perror("malloc");
        return NULL;
    }
    stm->states = socks5_states;
    stm->max_state = sizeof(socks5_states) / sizeof(socks5_states[0]);
    stm->initial = SOCKS5_HANDSHAKE;
    stm->current = NULL;
    LOG(DEBUG, "SOCKS5 state machine initialized with %d states", stm->max_state);
    return stm;
}

// --- Estados finales ---
static void handle_done(unsigned state, struct selector_key *key) {
    LOG(INFO, "SOCKS5 connection completed successfully for fd %d", key->fd);
    // Clean up resources if needed
    socks5_conn_t *conn = key->data;
    if (conn) {
        if (conn->origin_fd > 0) {
            LOG(DEBUG, "Closing origin connection fd %d", conn->origin_fd);
            close(conn->origin_fd);
        }
        LOG(DEBUG, "Cleaning up connection resources for fd %d", key->fd);
        // Additional cleanup can be added here
    }
    return;
}

static void handle_error(unsigned state, struct selector_key *key) {
    LOG(ERROR, "SOCKS5 connection ended with error for fd %d", key->fd);
    // Clean up resources on error
    socks5_conn_t *conn = key->data;
    if (conn) {
        if (conn->origin_fd > 0) {
            LOG(DEBUG, "Closing origin connection fd %d due to error", conn->origin_fd);
            close(conn->origin_fd);
        }
        LOG(DEBUG, "Cleaning up connection resources for fd %d due to error", key->fd);
        // Additional cleanup can be added here
    }
    return;
}