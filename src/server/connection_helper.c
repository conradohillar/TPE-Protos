#include <connection_helper.h>

int passive_accept(struct selector_key* key, void* data, const fd_handler* callback_functions) {

    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    const int client = accept4(key->fd, (struct sockaddr*) &client_addr, &client_addr_len, SOCK_NONBLOCK);
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
    struct sockaddr_storage addr;
    socklen_t addr_len = 0;

    int family = get_ip_family(address, port, &addr, &addr_len);
    if (family == -1) {
        LOG(ERROR, "Invalid address format: %s", address ? address : "(null)");
        *error_msg = "Invalid address format";
        return -1;
    }

    const int fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        LOG_MSG(ERROR, "Error creating socket");
        *error_msg = "Error creating socket";
        return -1;
    }

    LOG(INFO, "Server listening on TCP port %d - Address: %s - Protocol: %s", port, address, protocol);

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(fd, (struct sockaddr*) &addr, addr_len) < 0) {
        LOG_MSG(ERROR, "Error binding socket");
        *error_msg = "Error binding socket";
        goto error;
    }

    if (listen(fd, MAX_PENDING_CONNECTIONS) < 0) {
        LOG_MSG(ERROR, "Error listening on socket");
        *error_msg = "Error listening on socket";
        goto error;
    }

    *ss = set_non_blocking_fd(fd);
    if (*ss != SELECTOR_SUCCESS) {
        LOG_MSG(ERROR, "Error setting server socket to non-blocking");
        *error_msg = "Error setting server socket to non-blocking";
        goto error;
    }

    *ss = selector_register(*selector, fd, callback_functions, OP_READ, NULL);
    if (*ss != SELECTOR_SUCCESS) {
        LOG_MSG(ERROR, "Error registering server socket with selector");
        *error_msg = "Error registering server socket with selector";
        goto error;
    }

    return 0;

error:
    close(fd);
    return -1;
}

int get_ip_family(const char* ip_str, uint16_t port, struct sockaddr_storage* addr, socklen_t* addr_len) {
    if (ip_str == NULL) {
        struct sockaddr_in* addr4 = (struct sockaddr_in*) addr;
        memset(addr4, 0, sizeof(*addr4));
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        *addr_len = sizeof(struct sockaddr_in);
        return AF_INET;
    }

    struct sockaddr_in6* addr6 = (struct sockaddr_in6*) addr;
    memset(addr6, 0, sizeof(*addr6));
    if (inet_pton(AF_INET6, ip_str, &addr6->sin6_addr) == 1) {
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        *addr_len = sizeof(struct sockaddr_in6);
        return AF_INET6;
    }

    struct sockaddr_in* addr4 = (struct sockaddr_in*) addr;
    memset(addr4, 0, sizeof(*addr4));
    if (inet_pton(AF_INET, ip_str, &addr4->sin_addr) == 1) {
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        *addr_len = sizeof(struct sockaddr_in);
        return AF_INET;
    }

    return -1;
}
