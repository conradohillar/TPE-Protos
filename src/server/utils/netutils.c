#include <netutils.h>

#ifndef N
#define N(x) (sizeof(x) / sizeof((x)[0]))
#endif

extern const char*
sockaddr_to_human(char* buff, const size_t buffsize, const struct sockaddr* addr) {
    if (addr == 0) {
        strncpy(buff, "null", buffsize);
        return buff;
    }
    in_port_t port;
    void* p = 0x00;
    bool handled = false;

    switch (addr->sa_family) {
    case AF_INET:
        p = &((struct sockaddr_in*) addr)->sin_addr;
        port = ((struct sockaddr_in*) addr)->sin_port;
        handled = true;
        break;
    case AF_INET6:
        p = &((struct sockaddr_in6*) addr)->sin6_addr;
        port = ((struct sockaddr_in6*) addr)->sin6_port;
        handled = true;
        break;
    }
    if (handled) {
        if (inet_ntop(addr->sa_family, p, buff, buffsize) == 0) {
            strncpy(buff, "unknown ip", buffsize);
            buff[buffsize - 1] = 0;
        }
    } else {
        strncpy(buff, "unknown", buffsize);
    }

    strncat(buff, ":", buffsize);
    buff[buffsize - 1] = 0;
    const size_t len = strlen(buff);

    if (handled) {
        snprintf(buff + len, buffsize - len, "%d", ntohs(port));
    }
    buff[buffsize - 1] = 0;

    return buff;
}

int sock_blocking_write(const int fd, buffer* b) {
    int ret = 0;
    ssize_t nwritten;
    size_t n;
    uint8_t* ptr;

    do {
        ptr = buffer_read_ptr(b, &n);
        nwritten = send(fd, ptr, n, MSG_NOSIGNAL);
        if (nwritten > 0) {
            buffer_read_adv(b, nwritten);
        } else /* if (errno != EINTR) */ {
            ret = errno;
            break;
        }
    } while (buffer_can_read(b));

    return ret;
}

int sock_blocking_copy(const int source, const int dest) {
    int ret = 0;
    char buf[4096];
    ssize_t nread;
    while ((nread = recv(source, buf, N(buf), 0)) > 0) {
        char* out_ptr = buf;
        ssize_t nwritten;
        do {
            nwritten = send(dest, out_ptr, nread, MSG_NOSIGNAL);
            if (nwritten > 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else /* if (errno != EINTR) */ {
                ret = errno;
                goto error;
            }
        } while (nread > 0);
    }
error:

    return ret;
}

int set_non_blocking_fd(const int fd) {
    int ret = 0;
    int flags = fcntl(fd, F_GETFD, 0);
    if (flags == -1) {
        ret = -1;
    } else {
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            ret = -1;
        }
    }
    return ret;
}
void* resolve_host_name(void* arg) {
    socks5_conn_t* conn = (socks5_conn_t*) arg;

    struct addrinfo hints = {
        .ai_socktype = SOCK_STREAM,
        .ai_flags = 0,
    };

    if (conn->a_type == SOCKS5_CONN_REQ_ATYP_IPV4) {
        hints.ai_family = AF_INET;
        hints.ai_flags = AI_NUMERICHOST;
    } else if (conn->a_type == SOCKS5_CONN_REQ_ATYP_IPV6) {
        hints.ai_family = AF_INET6;
        hints.ai_flags = AI_NUMERICHOST;
    } else {
        hints.ai_family = AF_UNSPEC;
    }

    struct addrinfo* res;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", conn->dst_port);

    if (getaddrinfo((char*) conn->dst_address, port_str, &hints, &res) != 0) {
        LOG(ERROR, "Failed to resolve address: %s", conn->dst_address);
        return NULL;
    }

    LOG(DEBUG, "Resolved %s to address family %d", conn->dst_address, res->ai_family);

    conn->addr_info = res;
    selector_notify_block(conn->s, conn->client_fd);
    return NULL;
}


int connect_to_host(struct addrinfo** res, int* sock_fd) {
    struct addrinfo* rp = *res;
    while (rp != NULL) {
        *sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*sock_fd < 0) {
            LOG_MSG(DEBUG, "Failed to create socket, trying next address");
            return -1;
        }

        setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

        if (set_non_blocking_fd(*sock_fd) != SELECTOR_SUCCESS) {
            LOG(ERROR, "Failed to set socket %d to non-blocking mode", *sock_fd);
            close(*sock_fd);
            return -1;
        }
        if (connect(*sock_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            LOG(DEBUG, "Successfully connected to the target host for fd %d", *sock_fd);
            freeaddrinfo(*res);
            return CONNECTION_SUCCESS;
        } else if (errno == EINPROGRESS) {
            LOG(DEBUG, "Connection in progress for fd %d", *sock_fd);
            struct addrinfo* aux = rp->ai_next;
            rp->ai_next = NULL;
            freeaddrinfo(*res);
            *res = aux;
            return CONNECTION_IN_PROGRESS;
        }

        close(*sock_fd);
        rp = rp->ai_next;
    }
    if (*res != NULL) {
        freeaddrinfo(*res);
        *res = NULL;
    }
    return CONNECTION_FAILED;
}

void get_sock_data(int fd, uint8_t * atyp, void ** addr_ptr, uint16_t * port){
    struct sockaddr_storage local_addr;
    socklen_t addr_len = sizeof(local_addr);

    getsockname(fd, (struct sockaddr*) &local_addr, &addr_len);

    if (local_addr.ss_family == AF_INET) {
        struct sockaddr_in* addr_in = (struct sockaddr_in*) &local_addr;
        *atyp = ATYP_IPV4;
        *addr_ptr = &addr_in->sin_addr;
        *port = ntohs(addr_in->sin_port);
    } else if (local_addr.ss_family == AF_INET6) {
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*) &local_addr;
        *atyp = ATYP_IPV6;
        *addr_ptr = &addr_in6->sin6_addr;
        *port = ntohs(addr_in6->sin6_port);
    } 
}