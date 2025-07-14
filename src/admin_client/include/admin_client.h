#ifndef ADMIN_CLIENT_H
#define ADMIN_CLIENT_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <logger.h>
#include <unistd.h>

#define DEFAULT_SERVER_HOST "127.0.0.1"
#define DEFAULT_SERVER_PORT 8080

typedef enum response_code {
    RESPONSE_OK,
    RESPONSE_ERROR,
    RESPONSE_END,
    RESPONSE_PONG,
    RESPONSE_EMPTY,
    RESPONSE_BYE
} response_code_t;

// Conecta al servidor de administración, retorna el socket o -1 en error
int connect_to_admin_server(const char* host, int port);

// Loop principal del cliente administrador
void admin_client_loop(int sockfd);

#endif // ADMIN_CLIENT_H
