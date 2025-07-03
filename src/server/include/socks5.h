#ifndef SOCKS5NIO_H
#define SOCKS5NIO_H

#include <selector.h>
#include <sys/socket.h>
#include <handshake.h>
//#include <auth_wrapper.h>

#include <arpa/inet.h>
#include <assert.h> // assert
#include <connection_helper.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <unistd.h> // close

#define SOCKS5_BUFF_MAX_LEN 512

#define N(x) (sizeof(x) / sizeof((x)[0]))

/** maquina de estados general */
enum socks_v5state {
  /**
   * recibe el mensaje `hello` del cliente, y lo procesa
   *
   * Intereses:
   *     - OP_READ sobre client_fd
   *
   * Transiciones:
   *   - HELLO_READ  mientras el mensaje no esté completo
   *   - HELLO_WRITE cuando está completo
   *   - ERROR       ante cualquier error (IO/parseo)
   */
  HELLO_READ,

  /**
   * envía la respuesta del `hello' al cliente.
   *
   * Intereses:
   *     - OP_WRITE sobre client_fd
   *
   * Transiciones:
   *   - HELLO_WRITE  mientras queden bytes por enviar
   *   - REQUEST_READ cuando se enviaron todos los bytes
   *   - ERROR        ante cualquier error (IO/parseo)
   */
  HELLO_WRITE,
  // estados terminales
  DONE,
  ERROR,
};

typedef struct {
   /** File descriptor del cliente */
    int client_fd;
    /** File descriptor del origen (servidor destino) */
    int origin_fd;

    /** Máquina de estados principal */
    struct state_machine stm;

    handshake_parser *handshake_parser;
    auth_parser *auth_parser;

    // Manejo de buffers de la conexión
    uint8_t buf[SOCKS5_BUFF_MAX_LEN];
    size_t len;
    size_t parsed;
    uint8_t reply[SOCKS5_BUFF_MAX_LEN];
    size_t reply_len;
    size_t reply_sent;
    int status; // 0: esperando, 1: listo, -1: error

    /** Otros campos útiles */
    // dirección y puerto destino, métricas, etc.
} socks5_conn_t;

/** obtiene el struct (socks5 *) desde la llave de selección  */
#define ATTACHMENT(key) ((socks5_conn_t *)(key)->data)

void socksv5_passive_accept(struct selector_key *key);

#endif