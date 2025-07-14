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

socks5_conn_req_response create_conn_req_response(socks5_reply_status response, int fd){
    uint8_t atyp;
    char addr_ptr[INET6_ADDRSTRLEN];
    uint16_t port;
    
    get_sock_data(fd, &atyp, (void *)addr_ptr, &port);

    socks5_conn_req_response response_msg;
    response_msg.version = SOCKS5_VERSION;
    response_msg.response = (uint8_t)response;
    response_msg.reserved = SOCKS5_CONN_REQ_RSV;
    response_msg.address_type = atyp == AF_INET ? SOCKS5_CONN_REQ_ATYP_IPV4 : SOCKS5_CONN_REQ_ATYP_IPV6;

    if (atyp == AF_INET) {
        memcpy(response_msg.bnd_addr.ipv4.addr, addr_ptr, IPV4_ADDR_SIZE);
    } else if (atyp == AF_INET6) { 
        memcpy(response_msg.bnd_addr.ipv6.addr, addr_ptr, IPV6_ADDR_SIZE);
    } 

    response_msg.bnd_port = htons(port);
    return response_msg;
}

socks5_conn_req_response create_conn_req_error_response(socks5_reply_status response) {
    socks5_conn_req_response response_msg;
    response_msg.version = SOCKS5_VERSION;
    response_msg.response = (uint8_t)response;
    response_msg.reserved = SOCKS5_CONN_REQ_RSV;
    response_msg.address_type = SOCKS5_CONN_REQ_ATYP_IPV4;
    response_msg.bnd_addr.ipv4.addr[0] = 0;
    response_msg.bnd_addr.ipv4.addr[1] = 0;
    response_msg.bnd_addr.ipv4.addr[2] = 0;
    response_msg.bnd_addr.ipv4.addr[3] = 0;
    response_msg.bnd_port = 0;
    return response_msg;
}

