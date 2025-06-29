#include <connection_helper.h>

int passive_accept(struct selector_key *key, void * data, const fd_handler * callback_functions) {

    struct sockaddr client_addr;
    socklen_t client_addr_len;
    
    const int client = accept(key->fd, &client_addr, &client_addr_len);
    if(client == -1) goto fail;

    if(selector_fd_set_nio(client) == -1) goto fail;
    
    if(selector_register(key->s, client, callback_functions, OP_READ, data) != SELECTOR_SUCCESS) goto fail;
    
    return client;

    fail:
        if(client != -1) {
            close(client);
        }
    return -1;
}
