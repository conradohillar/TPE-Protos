#include <admin_client.h>

int connect_to_admin_server(const char *host, int port) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        struct hostent *he = gethostbyname(host);
        if (!he) { close(sockfd); return -1; }
        memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int receive_response(int sockfd, char *out, size_t outlen) {
    int outpos = 0;
    while (outpos < outlen - 1) {
        char c;
        ssize_t n = read(sockfd, &c, 1);
        if (n <= 0) break;
        out[outpos++] = c;
        if (c == '\n') break;
    }
    out[outpos] = '\0';
    return outpos;
}

void admin_client_loop(int sockfd) {
    char cmd[500];
    char resp[500];
    while (1) {
        printf(">: ");
        if (!fgets(cmd, sizeof(cmd), stdin)) 
            break;
        int n_write = write(sockfd, cmd, strlen(cmd));
        if (n_write <= 0) {
            printf("Error enviando comando\n");
            break;
        }
        int connection_closed = 0;
        do {
            int n_read = receive_response(sockfd, resp, sizeof(resp));
            if (n_read <= 0) {
                printf("\n[Conexión cerrada por el servidor]\n");
                connection_closed = 1;
                break;
            }
            printf("%s", resp);
            if (strstr(resp, "END") || strstr(resp, "OK") || strstr(resp, "ERROR") || strstr(resp, "PONG")) break;
            else if(strstr(resp, "BYE")) { return; }
        } while (1);
        if (connection_closed) break;
    }
}

int main(int argc, char *argv[]) {
    const char *host = DEFAULT_SERVER_HOST;
    int port = DEFAULT_SERVER_PORT;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) port = atoi(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0 && i+1 < argc) host = argv[++i];
    }
    int sockfd = connect_to_admin_server(host, port);
    if (sockfd < 0) {
        fprintf(stderr, "No se pudo conectar a %s:%d\n", host, port);
        return 1;
    }
    printf("[Conectado al servidor de administración]\n");
    admin_client_loop(sockfd);
    close(sockfd);
    return 0;
}