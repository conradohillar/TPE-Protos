#include <socks5_responses.h>


handshake_response create_handshake_response(uint8_t method) {
    handshake_response response;
    response.version = SOCKS5_VERSION;
    response.method = method;
    return response;
}

socks5_auth_response create_auth_response(bool auth_ok) {
    socks5_auth_response response;
    response.sub_negotiation_version = SOCKS5_SUBNEGOTIATION_VERSION;
    response.status = auth_ok ? SOCKS5_AUTH_OK : SOCKS5_AUTH_ERROR;
    return response;
}