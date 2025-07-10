#include <socks5_responses.h>


handshake_response create_handshake_response(uint8_t method) {
    handshake_response response;
    response.version = SOCKS5_VERSION;
    response.method = method;
    return response;
}