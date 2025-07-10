#ifndef __SOCKS5_RESPONSES_H__
#define __SOCKS5_RESPONSES_H__

#include <stdint.h>

#define HANDSHAKE_RESPONSE_SIZE 2
#define SOCKS5_VERSION 0x05
#define SOCKS5_AUTH_METHOD_NO_AUTH 0x00
#define SOCKS5_AUTH_METHOD_GSSAPI 0x01
#define SOCKS5_AUTH_METHOD_USER_PASS 0x02
#define SOCKS5_AUTH_METHOD_NO_ACCEPTABLE 0xFF



typedef struct  {
    uint8_t version; 
    uint8_t method;
}handshake_response;

handshake_response create_handshake_response(uint8_t method);

#endif