#include <metrics.h>

server_metrics_t *metrics_init(void) {
    server_metrics_t *server_metrics = malloc(sizeof(server_metrics_t));
    if (server_metrics == NULL) {
        log_error("Failed to allocate memory for metrics");
        perror("malloc");
        return NULL;
    }
    server_metrics->total_connections = 0;
    server_metrics->current_connections = 0;
    server_metrics->bytes_transferred = 0;
    server_metrics->errors = 0;
    log_info("Metrics system initialized");
    return server_metrics;
}

void metrics_inc_total_conn(server_metrics_t *server_metrics) {
    server_metrics->total_connections++;
    log_debug("Total connections incremented to %lu", server_metrics->total_connections);
}

void metrics_inc_curr_conn(server_metrics_t *server_metrics) {
    server_metrics->current_connections++;
    log_debug("Current connections incremented to %lu", server_metrics->current_connections);
}

void metrics_dec_curr_conn(server_metrics_t *server_metrics) {
    if (server_metrics->current_connections > 0) server_metrics->current_connections--;
    log_debug("Current connections decremented to %lu", server_metrics->current_connections);
}

void metrics_add_bytes(server_metrics_t *server_metrics, uint64_t bytes) {
    server_metrics->bytes_transferred += bytes;
    log_debug("Added %lu bytes, total transferred: %lu", bytes, server_metrics->bytes_transferred);
}

void metrics_inc_errors(server_metrics_t *server_metrics) {
    server_metrics->errors++;
    log_debug("Error count incremented to %lu", server_metrics->errors);
}

void metrics_print(server_metrics_t *server_metrics, char *buffer, size_t buffer_size) {
    if (buffer) {
        snprintf(buffer, buffer_size,
                 "Total Connections: %lu\n"
                 "Current Connections: %lu\n"
                 "Bytes Transferred: %lu\n"
                 "Errors: %lu\nEND\n",
                 server_metrics->total_connections,
                 server_metrics->current_connections,
                 server_metrics->bytes_transferred,
                 server_metrics->errors);
    }
}
