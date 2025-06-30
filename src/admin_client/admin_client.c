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

int send_command(int sockfd, const char *cmd) {
    size_t len = strlen(cmd);
    uint8_t data[1024];
    if (len + 2 > sizeof(data)) return -1;

    memcpy(data, cmd, len);
    data[len] = '\n';
    data[len+1] = 0; // null-terminator for safety
    
    buffer *b = (buffer *)malloc(sizeof(buffer));
    if (!b) return -1; // error allocating buffer
    buffer_init(b, len+1, data); // no null terminator in buffer
    b->write = b->data + len + 1; // incluye el '\n'
    int res = sock_blocking_write(sockfd, b);
    free(b);
    return res;
}

int receive_response(int sockfd, buffer *b, char *out, size_t outlen) {
    size_t outpos = 0;
    while (outpos < outlen - 1) {
        // Si hay datos en el buffer, leerlos
        size_t nbyte;
        uint8_t *ptr = buffer_read_ptr(b, &nbyte);
        if (nbyte > 0) {
            out[outpos] = *ptr;
            buffer_read_adv(b, 1);
            if (out[outpos++] == '\n') break;
            continue;
        }
        // Si no hay datos, leer del socket
        uint8_t c;
        ssize_t n = read(sockfd, &c, 1);
        if (n <= 0) break;
        buffer_write(b, c);
    }
    out[outpos] = '\0';
    return (int)outpos;
}

void admin_client_loop(int sockfd) {
    char cmd[MAX_CMD_LEN];
    char resp[MAX_RESPONSE_LEN];
    uint8_t bufdata[MAX_RESPONSE_LEN + 1];
    buffer b;
    buffer_init(&b, sizeof(bufdata), bufdata);
    while (1) {
        printf(">: ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        size_t len = strlen(cmd);
        if (len > 0 && cmd[len-1] == '\n') cmd[len-1] = '\0';
        if (send_command(sockfd, cmd) != 0) {
            printf("Error enviando comando\n");
            break;
        }
        // Recibir y mostrar respuesta (puede ser multilínea)
        int connection_closed = 0;
        do {
            int n = receive_response(sockfd, &b, resp, sizeof(resp));
            if (n <= 0) {
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