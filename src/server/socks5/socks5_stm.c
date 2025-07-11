#include <auth.h>
#include <conn_req.h>
#include <handshake.h>
#include <logger.h>
#include <socks5_copy.h>
#include <socks5_connecting.h>
#include <socks5_stm.h>
#include <stm.h>
#include <unistd.h>


static void handle_done(unsigned state, struct selector_key* key);
static void handle_error(unsigned state, struct selector_key* key);

struct state_definition socks5_states[] = {
    {
        .state = SOCKS5_HANDSHAKE,
        .on_arrival = handshake_on_arrival,
        .on_departure = handshake_on_departure,
        .on_read_ready = handshake_read,
        .on_write_ready = handshake_write
    },
    {
        .state = SOCKS5_AUTH,
        .on_arrival = auth_on_arrival,
        .on_departure = auth_on_departure,
        .on_read_ready = auth_read,
        .on_write_ready = auth_write
    },
    {
        .state = SOCKS5_CONNECTION_REQ,
        .on_arrival = connection_req_on_arrival,
        .on_departure = connection_req_on_departure,
        .on_read_ready = connection_req_read,
        .on_write_ready = connection_req_write
    },
    {
        .state = SOCKS5_CONNECTING,
        .on_block_ready = connecting_on_block_ready,
        .on_read_ready = connecting_read,
        .on_write_ready = connecting_write
    },
    {
        .state = SOCKS5_COPY,
        .on_arrival = copy_on_arrival,
        .on_read_ready = copy_read, 
        .on_departure = copy_on_departure
    },
    {
        .state = SOCKS5_DONE,
        .on_arrival = handle_done
    },
    {
        .state = SOCKS5_ERROR,
        .on_arrival = handle_error
    }
};

struct state_machine* socks5_stm_init(void) {
    struct state_machine* stm = malloc(sizeof(struct state_machine));
    if (stm == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for SOCKS5 state machine");
        return NULL;
    }
    stm->states = socks5_states;
    stm->max_state = sizeof(socks5_states) / sizeof(socks5_states[0]);
    stm->initial = SOCKS5_HANDSHAKE;
    stm->current = NULL;
    LOG(DEBUG, "SOCKS5 state machine initialized with %d states", stm->max_state);
    return stm;
}

static void handle_done(unsigned state, struct selector_key* key) {
    socks5_conn_t* conn = key->data;
    if (conn) {
        if (conn->origin_fd > 0) {
            LOG(DEBUG, "Closing origin connection fd %d", conn->origin_fd);
            selector_unregister_fd(key->s, conn->origin_fd);
        }
        if(conn->client_fd > 0) {
            LOG(DEBUG, "Closing client connection fd %d", conn->client_fd);
            selector_unregister_fd(key->s, conn->client_fd);
        }
    }
    return;
}

static void handle_error(unsigned state, struct selector_key* key) {
    LOG(ERROR, "SOCKS5 connection ended with error for fd %d", key->fd);
    socks5_conn_t* conn = key->data;
    if (conn != NULL) {
        if (conn->origin_fd > 0) {
            LOG(DEBUG, "Closing origin connection fd %d due to error", conn->origin_fd);
            selector_unregister_fd(key->s, conn->origin_fd);
        }
        if(conn->client_fd > 0) {
            LOG(DEBUG, "Closing client connection fd %d due to error", conn->client_fd);
            selector_unregister_fd(key->s, conn->client_fd);
        }
        LOG(DEBUG, "Cleaning up connection resources for fd %d due to error", key->fd);
    }
    return;
}

void socks5_stm_free(struct state_machine* stm) {
    if (stm != NULL) {
        LOG(DEBUG, "Freeing SOCKS5 state machine with %d states", stm->max_state);
        free(stm);
    } else {
        LOG_MSG(WARNING, "Attempted to free a NULL SOCKS5 state machine");
    }
}


bool has_write_handler(socks5_state state){
    if(state == SOCKS5_HANDSHAKE || state == SOCKS5_AUTH || state == SOCKS5_CONNECTION_REQ ||
       state == SOCKS5_CONNECTING) {
        return true;
    }
    return false;
}
