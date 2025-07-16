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

typedef enum response_code {
    RESPONSE_OK,
    RESPONSE_ERROR,
    RESPONSE_END,
    RESPONSE_PONG,
    RESPONSE_EMPTY,
    RESPONSE_BYE
} response_code_t;

int connect_to_admin_server(const char* host, int port);

void admin_client_loop(int sockfd);

#endif // ADMIN_CLIENT_H
