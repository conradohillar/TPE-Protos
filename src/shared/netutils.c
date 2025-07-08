#include <netutils.h>


#define N(x) (sizeof(x)/sizeof((x)[0]))

extern const char *
sockaddr_to_human(char *buff, const size_t buffsize,
                  const struct sockaddr *addr) {
    if(addr == 0) {
        strncpy(buff, "null", buffsize);
        return buff;
    }
    in_port_t port;
    void *p = 0x00;
    bool handled = false;

    switch(addr->sa_family) {
        case AF_INET:
            p    = &((struct sockaddr_in *) addr)->sin_addr;
            port =  ((struct sockaddr_in *) addr)->sin_port;
            handled = true;
            break;
        case AF_INET6:
            p    = &((struct sockaddr_in6 *) addr)->sin6_addr;
            port =  ((struct sockaddr_in6 *) addr)->sin6_port;
            handled = true;
            break;
    }
    if(handled) {
        if (inet_ntop(addr->sa_family, p,  buff, buffsize) == 0) {
            strncpy(buff, "unknown ip", buffsize);
            buff[buffsize - 1] = 0;
        }
    } else {
        strncpy(buff, "unknown", buffsize);
    }

    strncat(buff, ":", buffsize);
    buff[buffsize - 1] = 0;
    const size_t len = strlen(buff);

    if(handled) {
        snprintf(buff + len, buffsize - len, "%d", ntohs(port));
    }
    buff[buffsize - 1] = 0;

    return buff;
}


int
sock_blocking_write(const int fd, buffer *b) {
        int  ret = 0;
    ssize_t  nwritten;
	 size_t  n;
	uint8_t *ptr;

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

int
sock_blocking_copy(const int source, const int dest) {
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


int connect_to_host(const uint8_t *dst_addr, const uint16_t dst_port) {
    LOG(DEBUG, "Attempting to connect to host: %s:%u", dst_addr, dst_port);
    
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM
    };
    struct addrinfo *res;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", dst_port);

    if (getaddrinfo((char*)dst_addr, port_str, &hints, &res) != 0) {
        LOG(ERROR, "Failed to resolve hostname: %s", dst_addr);
        return -1; 
    }
    //TODO Falta liberar todos los structs
    LOG_MSG(DEBUG, "Hostname resolved, trying to connect");
    for (struct addrinfo *rp = res; rp != NULL; rp = rp->ai_next) {
        int sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) {
            LOG_MSG(DEBUG, "Failed to create socket, trying next address");
            continue;
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            LOG(INFO, "Successfully connected to %s:%u on socket %d", dst_addr, dst_port, sock);
            set_non_blocking_fd(sock);  
            freeaddrinfo(res);
            return sock;
        }

        LOG_MSG(DEBUG, "Connection failed, trying next address");
        close(sock);
    }

    LOG(ERROR, "Failed to connect to any address for %s:%u", dst_addr, dst_port);
    freeaddrinfo(res);
    return -1;
}