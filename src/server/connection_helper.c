#include <connection_helper.h>

int passive_accept(struct selector_key *key, void * data, const fd_handler * callback_functions) {

    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    const int client = accept(key->fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if(client == -1) goto fail;

    if(set_non_blocking_fd(client) == -1) goto fail;
    
    if(selector_register(key->s, client, callback_functions, OP_READ, data) != SELECTOR_SUCCESS) goto fail;
    
    return client;

    fail:
        if(client != -1) {
            close(client);
        }
    return -1;
}
