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
static void socksv5_done(struct selector_key *key);

static const struct fd_handler socks5_handler = {
    .handle_read = socksv5_read,
    .handle_write = socksv5_write,
    .handle_close = socksv5_close,
    .handle_block = socksv5_block,
};

/** Intenta aceptar la nueva conexión entrante*/
void socksv5_passive_accept(struct selector_key *key) {
  // crear el struct
  socks5_conn_t *conn = malloc(sizeof(socks5_conn_t));
  if (conn == NULL) {
    perror("malloc");
    return;
  }
  int fd = passive_accept(key, conn, &socks5_handler);
  if (fd == -1) {
    perror("passive_accept");
    free(conn);
    return;
  }
  conn->client_fd = fd;
  stm_init(conn->stm);

  key->data = conn;
}

static void socksv5_read(struct selector_key *key) {
  socks5_conn_t *conn = ATTACHMENT(key);
  ssize_t n = read(conn->client_fd, conn->buf + conn->len, sizeof(conn->buf) - conn->len);
  if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return SOCKS5_AUTH_READ; // No hay datos, esperar próximo evento
      } else {
          return SOCKS5_ERROR; // Error real
      }
  }
  if (n == 0) {
      return SOCKS5_ERROR; // EOF: el cliente cerró la conexión
  }
  conn->len += n;
  socks5_state state = stm_handler_read(conn->stm, key);
  if(state == SOCKS5_HANDSHAKE_READ || state == SOCKS5_AUTH_READ || state == SOCKS5_CONNECTION_REQ_READ) {
      // Continuar leyendo hasta que se complete el handshake o auth
      return;
  } else if(state == SOCKS5_HANDSHAKE_WRITE || state == SOCKS5_AUTH_WRITE || state == SOCKS5_CONNECTION_REQ_WRITE) {
      // Preparar para escribir la respuesta
      conn->reply_sent = 0;
      conn->reply_len = 0; // Resetear longitud de respuesta
      return;
  } else if(state == SOCKS5_DONE) {
      // Conexión completada, posiblemente iniciar relay de datos
      socksv5_done(key);
      return;
  } else if(state == SOCKS5_ERROR) {
      // Manejar error, posiblemente cerrando la conexión
      stm_handler_close(conn->stm, key);
      return;
  }
}

static void socksv5_write(struct selector_key *key) {
  socks5_conn_t *conn = ATTACHMENT(key);
  socks5_state state = stm_handler_write(conn->stm, key);
}

static void socksv5_block(struct selector_key *key) {
  socks5_conn_t *conn = ATTACHMENT(key);
  socks5_state state = stm_handler_block(conn->stm, key);
}

static void socksv5_close(struct selector_key *key) {
  socks5_conn_t *conn = ATTACHMENT(key);
  stm_handler_close(conn->stm, key);
}

static void socksv5_done(struct selector_key *key) {
  socks5_conn_t *conn = ATTACHMENT(key);
  stm_handler_done(conn->stm, key);
}
