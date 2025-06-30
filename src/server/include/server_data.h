#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <access_register.h>
#include <metrics.h>

typedef struct {
    // Metrics
    server_metrics_t *metrics;

    // Access Register
    access_register_t *access_register;

} server_data_t;

server_data_t *get_server_data();

#endif // SERVER_DATA_H