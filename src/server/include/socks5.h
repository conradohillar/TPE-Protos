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
#include <server.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
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

    uint8_t *in_buff_data;
    uint8_t *out_buff_data;

    char src_address[INET6_ADDRSTRLEN];
    uint16_t src_port;
    char username[MAX_USERNAME_LEN]; 

    char dst_address[DOMAIN_NAME_MAX_LENGTH];
    uint16_t dst_port;
    uint8_t a_type;

    struct addrinfo* addr_info;

    bool is_error_response;

} socks5_conn_t;

void socksv5_passive_accept(struct selector_key* key);
void socksv5_write(struct selector_key* key);

#endif
