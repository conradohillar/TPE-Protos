#include <connection_helper.h>

int passive_accept(struct selector_key* key, void* data, const fd_handler* callback_functions) {

    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    const int client = accept(key->fd, (struct sockaddr*) &client_addr, &client_addr_len);
    if (client == -1)
        goto fail;

    if (set_non_blocking_fd(client) == -1)
        goto fail;

    if (selector_register(key->s, client, callback_functions, OP_READ, data) != SELECTOR_SUCCESS)
        goto fail;

    return client;

fail:
    if (client != -1) {
        close(client);
    }
    return -1;
}

int create_and_register_passive_socket(fd_selector* selector, char* address, unsigned short port, const fd_handler* callback_functions, selector_status* ss, const char** error_msg, char* protocol) {
    struct addrinfo hints, *res = NULL;
    int fd = -1;
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%hu", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;            
    hints.ai_socktype = SOCK_STREAM;      
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(address, port_str, &hints, &res);
    if (err != 0) {
        LOG(ERROR, "getaddrinfo error: %s", gai_strerror(err));
        *error_msg = "Invalid address format";
        return -1;
    }
    struct addrinfo* p = res;
    while(p != NULL){

        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) {
            LOG_MSG(ERROR, "Error creating socket");
            *error_msg = "Error creating socket";
            goto cleanup;
        }

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

        if (bind(fd, p->ai_addr, p->ai_addrlen) < 0) {
            LOG_MSG(ERROR, "Error binding socket");
            *error_msg = "Error binding socket";
            goto cleanup;
        }

        if (listen(fd, MAX_PENDING_CONNECTIONS) < 0) {
            LOG_MSG(ERROR, "Error listening on socket");
            *error_msg = "Error listening on socket";
            goto cleanup;
        }

        char ip_str[INET6_ADDRSTRLEN] = {0};
        getnameinfo(p->ai_addr, p->ai_addrlen, ip_str, sizeof(ip_str), NULL, 0, NI_NUMERICHOST);

        LOG(INFO, "Server listening on TCP port %d - Address %s - Protocol: %s", port, ip_str, protocol);


        *ss = set_non_blocking_fd(fd);
        if (*ss != SELECTOR_SUCCESS) {
            LOG_MSG(ERROR, "Error setting socket to non-blocking");
            *error_msg = "Error setting socket to non-blocking";
            goto cleanup;
        }

        *ss = selector_register(*selector, fd, callback_functions, OP_READ, NULL);
        if (*ss != SELECTOR_SUCCESS) {
            LOG_MSG(ERROR, "Error registering socket with selector");
            *error_msg = "Error registering socket with selector";
            goto cleanup;
        }
        p = p->ai_next;
    }

    freeaddrinfo(res);
    return 0;

cleanup:
    if (fd >= 0) close(fd);
    if (res != NULL) freeaddrinfo(res);
    return -1;
}
