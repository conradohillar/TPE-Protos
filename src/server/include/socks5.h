#ifndef SOCKS5NIO_H
#define SOCKS5NIO_H

#include <selector.h>
#include <sys/socket.h>
#include <stm.h>
#include <socks5_stm.h>
#include <connection_helper.h>
#include <buffer.h>

#include <arpa/inet.h>
#include <assert.h> // assert
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <unistd.h> // close

#define SOCKS5_BUFF_MAX_LEN 512

#define N(x) (sizeof(x) / sizeof((x)[0]))

typedef struct {
   /** File descriptor del cliente */
    int client_fd;
    /** File descriptor del origen (servidor destino) */
    int origin_fd;

    /** Máquina de estados principal */
    struct state_machine *stm;

    handshake_parser *handshake_parser;
    auth_parser *auth_parser;
    conn_req_parser *conn_req_parser;

    // Manejo de buffers de la conexión
    buffer in_buff;
    buffer out_buff;

    /** Otros campos útiles */
    // dirección y puerto destino, métricas, etc.
} socks5_conn_t;

/** obtiene el struct (socks5 *) desde la llave de selección  */
#define ATTACHMENT(key) ((socks5_conn_t *)(key)->data)

void socksv5_passive_accept(struct selector_key *key);

#endif