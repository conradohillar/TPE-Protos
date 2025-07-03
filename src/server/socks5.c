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

#include <errno.h>
#include <stdio.h>

static void socksv5_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    if (!buffer_can_write(&conn->in_buff)) {      //Puede pasar que este lleno el buffer de entrada, cerramos la conexion o que?
        selector_set_interest_key(key, OP_NOOP);  //Por ahora no hago nada, hago que quede el socket muerto y despues vemos igual no deberia pasar este caso creo
        return;
    }

    size_t n;
    uint8_t *write_ptr = buffer_write_ptr(&conn->in_buff, &n);

    ssize_t n_read = recv(key->fd, write_ptr, n, MSG_DONTWAIT);  

    if (n_read > 0) {
        buffer_write_adv(&conn->in_buff, n_read);
        stm_handler_read(conn->stm, key);
        //VER CON EZE, aca creo que tengo que ver que retorno y en funcion de eso setear el interes del fd

    } else if (n_read == 0) {
        selector_unregister_fd(key->s, key->fd);    // ACA RECIBIMOS EOF, CREO QUE DEBERIAMOS LIBERAR LOS RECURSOS
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            //tenemos algun otro error, tenemos que notificar al cliente, liberamos y cerreamos todp
        }
    }
}


static void socksv5_write(struct selector_key *key) {
    socks5_conn_t *conn = key->data;

    if (!buffer_can_read(&conn->out_buff)) {
        selector_set_interest_key(key, OP_NOOP);
        return;
    }

    size_t n;
    uint8_t *read_ptr = buffer_read_ptr(&conn->out_buff, &n);

    ssize_t n_written = send(key->fd, read_ptr, n, MSG_DONTWAIT); 

    if (n_written > 0) {
        buffer_read_adv(&conn->out_buff, n_written);
        if (!buffer_can_read(&conn->out_buff)) {
            selector_set_interest_key(key, OP_READ); // Si no hay mas para darle al cliente supongo que esta bien decir que ahora queremos leer
        }
    } else if (n_written < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            
        }
        //el else de esto seria dejar el estado como esta para que cuando el clilente libere espacio el fd le mande lo que faltaba
    }
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
  
}
