#ifndef __SOCKS5_RESPONSES_H__
#define __SOCKS5_RESPONSES_H__

#include <stdint.h>
#include <stdbool.h>

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


typedef struct  {
    uint8_t version; 
    uint8_t method;
}handshake_response;

typedef struct {
    uint8_t sub_negotiation_version;
    uint8_t status; 
} socks5_auth_response;

handshake_response create_handshake_response(uint8_t method);
socks5_auth_response create_auth_response(bool auth_ok);
#endif