#ifndef __SOCKS5_RESPONSES_H__
#define __SOCKS5_RESPONSES_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <netutils.h>

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

#define IPV4_ADDR_SIZE 4
#define IPV6_ADDR_SIZE 16

#define SOCKS5_CONN_REQ_RESPONSE_BASE_SIZE 6
typedef enum {
    SOCKS5_REP_SUCCEEDED              = 0x00,
    SOCKS5_REP_GENERAL_FAILURE        = 0x01,
    SOCKS5_REP_CONNECTION_NOT_ALLOWED = 0x02,
    SOCKS5_REP_NETWORK_UNREACHABLE    = 0x03,
    SOCKS5_REP_HOST_UNREACHABLE       = 0x04,
    SOCKS5_REP_CONNECTION_REFUSED     = 0x05,
    SOCKS5_REP_TTL_EXPIRED            = 0x06,
    SOCKS5_REP_COMMAND_NOT_SUPPORTED  = 0x07,
    SOCKS5_REP_ADDR_TYPE_NOT_SUPPORTED= 0x08,
    SOCKS5_REP_UNASSIGNED             = 0x09 
} socks5_reply_status;


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
            uint8_t addr[IPV4_ADDR_SIZE];
        } ipv4;
        struct {
            uint8_t addr[IPV6_ADDR_SIZE]; 
        } ipv6;
    } bnd_addr;
    uint16_t bnd_port; 
} socks5_conn_req_response;

socks5_handshake_response create_handshake_response(bool no_auth, bool user_pass_auth);
socks5_auth_response create_auth_response(bool auth_ok);
socks5_conn_req_response create_conn_req_response(socks5_reply_status response, int fd);
socks5_conn_req_response create_conn_req_error_response(socks5_reply_status response);


#endif
