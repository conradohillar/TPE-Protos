/**
 * socks5.c - accepta conexiones y controla el flujo de un proxy SOCKSv5
 * (sockets no bloqueantes)
 */

#include <socks5.h>

/* declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void socksv5_read(struct selector_key *key);
static void socksv5_write(struct selector_key *key);
static void socksv5_block(struct selector_key *key);
static void socksv5_close(struct selector_key *key);

static const struct fd_handler socks5_handler = {
    .handle_read = socksv5_read,
    .handle_write = socksv5_write,
    .handle_close = socksv5_close,
    .handle_block = socksv5_block,
};

/** Intenta aceptar la nueva conexión entrante*/
void socksv5_passive_accept(struct selector_key *key) {
  // crear el struct
  int fd = passive_accept(key, NULL, &socks5_handler);
}

static void socksv5_done(struct selector_key *key);

static void socksv5_read(struct selector_key *key) {}

static void socksv5_write(struct selector_key *key) {}

static void socksv5_block(struct selector_key *key) {}

static void socksv5_close(struct selector_key *key) {}

static void socksv5_done(struct selector_key *key) {}
