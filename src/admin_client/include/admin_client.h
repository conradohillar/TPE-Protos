#ifndef ADMIN_CLIENT_H
#define ADMIN_CLIENT_H

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../config.h"
#include "../../selector.h"

#define DEFAULT_SERVER_HOST DEFAULT_CONFIG_HOST
#define DEFAULT_SERVER_PORT DEFAULT_CONFIG_PORT
#define BACKLOG 5

// Estructura de estado para multiplexado
typedef struct {
    int sockfd;
    int connected;
    char outbuf[MAX_CMD_LEN];
    size_t outbuf_len;
    size_t outbuf_sent;
    char inbuf[MAX_RESPONSE_LEN];
    size_t inbuf_len;
} admin_client_state_t;

// Conecta al servidor de administración, retorna el socket o -1 en error
int connect_to_admin_server(const char *host, int port);

// Loop principal del cliente administrador
void admin_client_loop(int sockfd);

// Libera los recursos del estado del cliente
void release_resources(admin_client_state_t *state);

#endif // ADMIN_CLIENT_H
