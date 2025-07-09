#ifndef SOCKS5NIO_H
#define SOCKS5NIO_H

#include <selector.h>
#include <handshake_parser.h>
#include <auth_parser.h>
#include <conn_req_parser.h>
#include <buffer.h>
#include <socks5_stm.h>
#include <connection_helper.h>
#include <logger.h>
#include <stm.h>
#include <metrics.h>
#include <main.h>

#include <arpa/inet.h>
#include <assert.h> // assert
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <unistd.h> // close
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define SOCKS5_BUFF_MAX_LEN 512

#define N(x) (sizeof(x) / sizeof((x)[0]))

#define DOMAIN_NAME_MAX_LENGTH 254

typedef struct {
    fd_selector s;
    int client_fd;
    int origin_fd;

    struct state_machine* stm;

    handshake_parser* handshake_parser;
    auth_parser* auth_parser;
    conn_req_parser* conn_req_parser;

    buffer in_buff;
    buffer out_buff;

    uint8_t in_buff_data[SOCKS5_BUFF_MAX_LEN];
    uint8_t out_buff_data[SOCKS5_BUFF_MAX_LEN];

    char dst_address[DOMAIN_NAME_MAX_LENGTH];
    uint16_t dst_port;

    struct addrinfo* addr_info;

} socks5_conn_t;

void socksv5_passive_accept(struct selector_key* key);

#endif