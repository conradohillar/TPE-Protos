#ifndef __SOCKS5_RESPONSES_H__
#define __SOCKS5_RESPONSES_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

#define HANDSHAKE_RESPONSE_SIZE 2
#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_METHOD_NO_AUTH 0x00
#define SOCKS5_AUTH_METHOD_GSSAPI 0x01
#define SOCKS5_AUTH_METHOD_USER_PASS 0x02
#define SOCKS5_AUTH_METHOD_NO_ACCEPTABLE 0xFF

#define AUTH_RESPONSE_SIZE 2
#define SOCKS5_SUBNEGOTIATION_VERSION 0x01
#define SOCKS5_AUTH_OK 0x00
#define SOCKS5_AUTH_ERROR 0x01

#define SOCKS5_CONN_REQ_CMD_CONNECT 0x01
#define SOCKS5_CONN_REQ_CMD_BIND 0x02
#define SOCKS5_CONN_REQ_CMD_UDP_ASSOCIATE 0x03
#define SOCKS5_CONN_REQ_RSV 0x00
#define SOCKS5_CONN_REQ_ATYP_IPV4 0x01
#define SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME 0x03
#define SOCKS5_CONN_REQ_ATYP_IPV6 0x04


typedef struct  {
    uint8_t version; 
    uint8_t method;
} socks5_handshake_response;

typedef struct {
    uint8_t sub_negotiation_version;
    uint8_t status; 
} socks5_auth_response;

typedef struct {
    uint8_t version;
    uint8_t response;
    uint8_t reserved;
    uint8_t address_type;
    union {
        struct {
            uint8_t addr[4];
        } ipv4;
        struct {
            uint8_t addr[16]; 
        } ipv6;
        struct {
            uint8_t len;
            uint8_t name[255];
        } domain;
    } bnd_addr;
    uint16_t bnd_port; 
} socks5_conn_req_response;

socks5_handshake_response create_handshake_response(bool no_auth, bool user_pass_auth);
socks5_auth_response create_auth_response(bool auth_ok);
socks5_conn_req_response create_conn_req_response(uint8_t response, uint8_t address_type, const void* addr, uint16_t port);
#endif