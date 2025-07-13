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



int connect_to_host(struct addrinfo* res) {
    int sock_fd = -1;
    struct addrinfo* rp = res;
    while (rp != NULL) {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock_fd < 0) {
            LOG_MSG(DEBUG, "Failed to create socket, trying next address");
            return -1;
        }

        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            LOG(DEBUG, "Successfully connected to the target host for fd %d", sock_fd);
            return sock_fd;
        } 

        close(sock_fd);
        rp = rp->ai_next;
    }
    return -1;
}

int connect_to_admin_server(const char* host, int port) {
    struct addrinfo* res;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", port);
    
    struct addrinfo hints = {
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AF_UNSPEC,
    };

    if (getaddrinfo((char*) host, port_str, &hints, &res) != 0) {
        LOG(ERROR, "Failed to resolve address: %s", host);
        return -1;
    }
    int sock_fd = connect_to_host(res);
    freeaddrinfo(res);
    if (sock_fd < 0) {
        LOG(ERROR, "Failed to connect to %s:%d", host, port);
        return -1;  
    }
    LOG(DEBUG, "Connected to %s:%d", host, port);
    return sock_fd;
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
