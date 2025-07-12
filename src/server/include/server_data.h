#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <access_register.h>
#include <metrics.h>
#include <defines.h>

typedef struct {
    // Metrics
    server_metrics_t* metrics;

    // Access Register
    access_register_t* access_register;

    // Connection buffers size
    unsigned int buffer_size;

    // Maximum number of connections
    unsigned int max_conn;

} server_data_t;

server_data_t* get_server_data(void);

void server_data_init(void);

void server_data_destroy(void);

int set_buffer_size(int size);

int set_max_conn(int max_conn);

#endif // SERVER_DATA_H
