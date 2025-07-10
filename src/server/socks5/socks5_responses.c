#include <socks5_responses.h>


socks5_handshake_response create_handshake_response(bool no_auth, bool user_pass_auth) {
    socks5_handshake_response response;
    response.version = SOCKS5_VERSION;
    response.method = user_pass_auth ? SOCKS5_AUTH_METHOD_USER_PASS : (no_auth ? SOCKS5_AUTH_METHOD_NO_AUTH : SOCKS5_AUTH_METHOD_NO_ACCEPTABLE);
    return response;
}

socks5_auth_response create_auth_response(bool auth_ok) {
    socks5_auth_response response;
    response.sub_negotiation_version = SOCKS5_SUBNEGOTIATION_VERSION;
    response.status = auth_ok ? SOCKS5_AUTH_OK : SOCKS5_AUTH_ERROR;
    return response;
}

socks5_conn_req_response create_conn_req_response(uint8_t response, uint8_t address_type, const void* addr, uint16_t port) {
    socks5_conn_req_response response_msg;
    response_msg.version = SOCKS5_VERSION;
    response_msg.response = response;
    response_msg.reserved = SOCKS5_CONN_REQ_RSV;
    response_msg.address_type = address_type;

    if (address_type == SOCKS5_CONN_REQ_ATYP_IPV4) {
        memcpy(response_msg.bnd_addr.ipv4.addr, addr, 4);
    } else if (address_type == SOCKS5_CONN_REQ_ATYP_IPV6) { 
        memcpy(response_msg.bnd_addr.ipv6.addr, addr, 16);
    } else if (address_type == SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME) { 
        response_msg.bnd_addr.domain.len = ((uint8_t*)addr)[0];
        memcpy(response_msg.bnd_addr.domain.name, (uint8_t*)addr + 1, response_msg.bnd_addr.domain.len);
    }

    response_msg.bnd_port = htons(port);
    return response_msg;
}