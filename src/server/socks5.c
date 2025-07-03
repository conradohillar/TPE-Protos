/**
 * socks5.c - accepta conexiones y controla el flujo de un proxy SOCKSv5
 * (sockets no bloqueantes)
 */

#include <socks5.h>
#include <socks5_stm.h>

/* declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void socksv5_read(struct selector_key *key);
static void socksv5_write(struct selector_key *key);
static void socksv5_block(struct selector_key *key);
static void socksv5_close(struct selector_key *key);
static void socksv5_done(struct selector_key *key);

static const struct fd_handler socks5_handler = {
    .handle_read = socksv5_read,
    .handle_write = socksv5_write,
    .handle_close = socksv5_close,
    .handle_block = socksv5_block,
};

extern struct state_machine socks5_stm; // (declarado en socks5_stm.c)

/** Intenta aceptar la nueva conexión entrante*/
void socksv5_passive_accept(struct selector_key *key) {
  // crear el struct
  socks5_conn_t *conn = malloc(sizeof(socks5_conn_t));
  if (conn == NULL) {
    perror("malloc");
    return;
  }
  key->data = conn;

  int fd = passive_accept(key, conn, &socks5_handler);
  if (fd == -1) {
    perror("passive_accept");
    free(conn);
    return;
  }
  conn->client_fd = fd;

  conn->stm = &socks5_stm;
  stm_init(conn->stm);
}

static void socksv5_read(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
}

static void socksv5_write(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  socks5_state state = stm_handler_write(conn->stm, key);
}

static void socksv5_block(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  socks5_state state = stm_handler_block(conn->stm, key);
}

static void socksv5_close(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  stm_handler_close(conn->stm, key);
}

static void socksv5_done(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  stm_handler_done(conn->stm, key);
}
