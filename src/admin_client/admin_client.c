#include <admin_client.h>
#include <args_admin.h>
static char * _responses[] = {
    "OK",
    "ERROR",
    "END",
    "PONG",
    "",
    "BYE"
};


int connect_to_admin_server(const char* host, int port) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        struct hostent* he = gethostbyname(host);
        if (!he) {
            close(sockfd);
            return -1;
        }
        memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

void admin_client_loop(int sockfd) {
    char cmd[MAX_CMD_LEN];
    char resp[MAX_RESPONSE_LEN];
    while (1) {
        printf(">: ");
        if (!fgets(cmd, sizeof(cmd), stdin))
            break;
        size_t n_write = write(sockfd, cmd, strlen(cmd));
        if (n_write <= 0) {
            printf("Error enviando comando\n");
            break;
        }
        int connection_closed = 0, response_found = 0;
        do {
            size_t n_read = read(sockfd, resp, sizeof(resp));
            if (n_read <= 0) {
                printf("\n[Conexión cerrada por el servidor]\n");
                connection_closed = 1;
                break;
            }
            resp[n_read] = '\0';
            printf("%s", resp);
            if (strstr(resp, _responses[RESPONSE_BYE])) {
                return;
            }
            for(unsigned int i = 0; !response_found && i < (sizeof(_responses) / sizeof(_responses[0])) - 1; i++) {
                response_found = strstr(resp, _responses[i]) != NULL;
            }
        } while (!response_found);
        
        if (connection_closed)
            break;
    }
}

int main(int argc, char* argv[]) {
    admin_args args;
    parse_args(argc, argv, &args);

    init_logging(args.log_file, args.log_level);
    int sockfd = connect_to_admin_server(args.host, args.port);
    if (sockfd < 0) {
        LOG(ERROR, "No se pudo conectar a %s:%d", args.host, args.port);
        return 1;
    }
    LOG_MSG(INFO, "Conectado al servidor de administración");
    admin_client_loop(sockfd);
    close(sockfd);
    close_logging();
    return 0;
}
