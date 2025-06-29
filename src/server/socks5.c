/**
 * socks5.c - accepta conexiones y controla el flujo de un proxy SOCKSv5 (sockets no bloqueantes)
 */

#include <socks5.h>

#include <stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>  // memset
#include <assert.h>  // assert
#include <errno.h>
#include <time.h>
#include <unistd.h>  // close
#include <pthread.h>
#include <arpa/inet.h>
#include <selector.h>
#include <connection_helper.h>


#define N(x) (sizeof(x)/sizeof((x)[0]))


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

struct socks5 {
    /** maquinas de estados */
    // struct state_machine          stm;

    /** estados para el client_fd */
    union {
        // struct hello_st           hello;
        // struct request_st         request;
        // struct copy               copy;
    } client;
    /** estados para el origin_fd */
    union {
        // struct connecting         conn;
        // struct copy               copy;
    } orig;

};



/** obtiene el struct (socks5 *) desde la llave de selección  */
#define ATTACHMENT(key) ( (struct socks5 *)(key)->data)

/* declaración forward de los handlers de selección de una conexión
 * establecida entre un cliente y el proxy.
 */
static void socksv5_read   (struct selector_key *key);
static void socksv5_write  (struct selector_key *key);
static void socksv5_block  (struct selector_key *key);
static void socksv5_close  (struct selector_key *key);

static const struct fd_handler socks5_handler = {
    .handle_read   = socksv5_read,
    .handle_write  = socksv5_write,
    .handle_close  = socksv5_close,
    .handle_block  = socksv5_block,
};

/** Intenta aceptar la nueva conexión entrante*/
void socksv5_passive_accept(struct selector_key *key) {
    //crear el struct
    int fd = passive_accept(key, NULL, &socks5_handler);
}


static void
socksv5_done(struct selector_key* key);

static void
socksv5_read(struct selector_key *key) {
    
}

static void
socksv5_write(struct selector_key *key) {
    
}

static void
socksv5_block(struct selector_key *key) {
    
}

static void
socksv5_close(struct selector_key *key) {
    
}

static void
socksv5_done(struct selector_key* key) {
    
}
