#ifndef ADMIN_CLIENT_H
#define ADMIN_CLIENT_H

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netutils.h>
#include <stdlib.h>

#define DEFAULT_SERVER_HOST DEFAULT_CONFIG_HOST
#define DEFAULT_SERVER_PORT DEFAULT_CONFIG_PORT

// Conecta al servidor de administración, retorna el socket o -1 en error
int connect_to_admin_server(const char *host, int port);

// Envía un comando al servidor usando buffer y sock_blocking_write
int send_command(int sockfd, const char *cmd);

// Recibe una línea de respuesta del servidor usando buffer
int receive_response(int sockfd, buffer *b, char *out, size_t outlen);

// Loop principal del cliente administrador
void admin_client_loop(int sockfd);

#endif // ADMIN_CLIENT_H