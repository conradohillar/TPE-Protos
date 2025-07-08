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
  log_debug("Trying to accept a new SOCKSv5 connection");
  // crear el struct
  socks5_conn_t *conn = malloc(sizeof(socks5_conn_t));
  if (conn == NULL) {
    log_error("Failed to allocate memory for SOCKSv5 connection");
    perror("malloc");
    return;
  }
  key->data = conn;

  int fd = passive_accept(key, conn, &socks5_handler);
  if (fd == -1) {
    log_error("Failed to accept SOCKSv5 connection");
    perror("passive_accept");
    free(conn);
    return;
  }
  conn->client_fd = fd;
  buffer_init(&conn->in_buff, conn->in_buff_data, conn->in_buff_data);
  buffer_init(&conn->out_buff, conn->out_buff_data, conn->out_buff_data);
  conn->stm = socks5_stm_init();

  log_info("New SOCKSv5 connection accepted on fd %d", fd);

  metrics_inc_curr_conn(get_server_data()->metrics);
  metrics_inc_total_conn(get_server_data()->metrics);
}

static void socksv5_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    if (!buffer_can_write(&conn->in_buff)) {      //Puede pasar que este lleno el buffer de entrada, cerramos la conexion o que?
        log_warning("Input buffer full for fd %d, setting NOOP", key->fd);
        selector_set_interest_key(key, OP_NOOP);  //Por ahora no hago nada, hago que quede el socket muerto y despues vemos igual no deberia pasar este caso creo
        return;
    }

    size_t n;
    uint8_t *write_ptr = buffer_write_ptr(&conn->in_buff, &n);

    ssize_t n_read = recv(key->fd, write_ptr, n, MSG_DONTWAIT);  

    if (n_read > 0) {
        log_debug("Read %zd bytes from fd %d", n_read, key->fd);
        buffer_write_adv(&conn->in_buff, n_read);
        socks5_state state = stm_handler_read(conn->stm, key);
        if(state == SOCKS5_ERROR){
          log_error("Error in SOCKS5 state machine for fd %d", key->fd);
          metrics_inc_errors(get_server_data()->metrics);
        }
        
    } else if (n_read == 0) {
        log_info("Connection closed by client on fd %d", key->fd);
        selector_unregister_fd(key->s, key->fd);    
        // ACA RECIBIMOS EOF, CREO QUE DEBERIAMOS LIBERAR LOS RECURSOS
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_error("Error reading from fd %d: %s", key->fd, strerror(errno));
            metrics_inc_errors(get_server_data()->metrics);
            // TODO: liberar recursos
        }
    }
}


static void socksv5_write(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    if (!buffer_can_read(&conn->out_buff)) {
        log_debug("Output buffer empty for fd %d, setting NOOP", key->fd);
        selector_set_interest_key(key, OP_NOOP);
        return;
    }

    size_t n;
    uint8_t *read_ptr = buffer_read_ptr(&conn->out_buff, &n);

    ssize_t n_written = send(key->fd, read_ptr, n, MSG_DONTWAIT); 

    if (n_written > 0) {
        log_debug("Wrote %zd bytes to fd %d", n_written, key->fd);
        metrics_add_bytes(get_server_data()->metrics, n_written);
        buffer_read_adv(&conn->out_buff, n_written);
        if (!buffer_can_read(&conn->out_buff)) {
            selector_set_interest_key(key, OP_READ); // Si no hay mas para darle al cliente supongo que esta bien decir que ahora queremos leer 
            buffer_reset(&conn->out_buff); // Reseteamos el buffer de salida
        }
        if (buffer_can_write(&conn->in_buff)) {
            selector_set_interest_key(key, OP_READ); // Si hay espacio en el buffer de entrada
        }
    } else if (n_written < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_error("Error writing to fd %d: %s", key->fd, strerror(errno));
            metrics_inc_errors(get_server_data()->metrics);
            // TODO: liberar recursos
        }
    }
}



static void socksv5_block(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  log_debug("Handling block event for fd %d", key->fd);
  socks5_state state = stm_handler_block(conn->stm, key);
  if (state == SOCKS5_ERROR) {
    log_error("Error in SOCKS5 state machine block handler for fd %d", key->fd);
    metrics_inc_errors(get_server_data()->metrics);
    // TODO: liberar recursos
  }
}

static void socksv5_close(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  log_info("Closing SOCKS5 connection on fd %d", key->fd);
  stm_handler_close(conn->stm, key);
  metrics_dec_curr_conn(get_server_data()->metrics);
}

static void socksv5_done(struct selector_key *key) {
  socks5_conn_t *conn = key->data;
  // stm_handler_done(conn->stm, key);
}
