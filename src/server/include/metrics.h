#ifndef METRICS_H
#define METRICS_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <logger.h>

typedef struct {
    uint64_t total_connections;
    uint64_t current_connections;
    uint64_t bytes_transferred;
    uint64_t errors;
} server_metrics_t;

server_metrics_t* metrics_init(void);

void metrics_inc_total_conn(server_metrics_t* server_metrics);
void metrics_inc_curr_conn(server_metrics_t* server_metrics);
void metrics_dec_curr_conn(server_metrics_t* server_metrics);
void metrics_add_bytes(server_metrics_t* server_metrics, uint64_t bytes);
void metrics_inc_errors(server_metrics_t* server_metrics);

void metrics_print(server_metrics_t* server_metrics, char* buffer, size_t buffer_size);

#endif // METRICS_H
