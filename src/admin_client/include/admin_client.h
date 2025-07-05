#ifndef ADMIN_CLIENT_H
#define ADMIN_CLIENT_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netutils.h>
#include <stdlib.h>

#define DEFAULT_SERVER_HOST "127.0.0.1"
#define DEFAULT_SERVER_PORT 8080

// Conecta al servidor de administración, retorna el socket o -1 en error
int connect_to_admin_server(const char *host, int port);

// Recibe una línea de respuesta del servidor
int receive_response(int sockfd, char *out, size_t outlen);

// Loop principal del cliente administrador
void admin_client_loop(int sockfd);

#endif // ADMIN_CLIENT_H