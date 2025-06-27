#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "config.h"

#define DEFAULT_PORT DEFAULT_CONFIG_PORT
#define BACKLOG 5

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) port = atoi(argv[++i]);
    }
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind"); close(listenfd); exit(1);
    }
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen"); close(listenfd); exit(1);
    }
    printf("Servidor de prueba escuchando en puerto %d...\n", port);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
        if (connfd < 0) { perror("accept"); continue; }
        printf("Cliente conectado!\n");
        // Lee y responde múltiples líneas hasta que el cliente cierre la conexión
        char buf[MAX_RESPONSE_LEN];
        while (1) {
            ssize_t n = read(connfd, buf, sizeof(buf)-1);
            if (n <= 0) break;
            buf[n] = '\0';
            config_handler(connfd, buf); // Maneja el comando recibido
        }
        close(connfd);
    }
    close(listenfd);
    return 0;
}
