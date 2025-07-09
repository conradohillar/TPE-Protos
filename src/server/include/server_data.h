#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <access_register.h>
#include <metrics.h>
#include <defines.h>

typedef struct {
    // Metrics
    server_metrics_t *metrics;

    // Access Register
    access_register_t *access_register;
    
    // Connection buffers size
    int buffer_size;

    // Timeout for connections in seconds
    int timeout;

} server_data_t;

server_data_t *get_server_data();

void server_data_init();

void server_data_destroy();

int set_buffer_size(int size);

int set_timeout(int seconds);

#endif // SERVER_DATA_H