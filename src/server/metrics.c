#include <metrics.h>

server_metrics_t *metrics_init() {
    server_metrics_t *metrics = calloc(1, sizeof(server_metrics_t));
    if (!metrics) {
        fprintf(stderr, "Failed to allocate memory for server metrics");
        exit(EXIT_FAILURE);
    }
    metrics->total_connections = 0;
    metrics->current_connections = 0;
    metrics->bytes_transferred = 0;
    metrics->errors = 0;
    return metrics;
}

void metrics_inc_total_connections(server_metrics_t *metrics) {
    if (metrics) metrics->total_connections++;
}

void metrics_inc_current_connections(server_metrics_t *metrics) {
    if (metrics) metrics->current_connections++;
}

void metrics_dec_current_connections(server_metrics_t *metrics) {
    if (metrics && metrics->current_connections > 0) metrics->current_connections--;
}

void metrics_add_bytes(server_metrics_t *metrics, uint64_t bytes) {
    if (metrics) metrics->bytes_transferred += bytes;
}

void metrics_inc_errors(server_metrics_t *metrics) {
    if (metrics) metrics->errors++;
}

void metrics_print(const server_metrics_t *metrics, char *buffer, size_t buffer_size) {
    if (metrics && buffer) {
        snprintf(buffer, buffer_size,
                 "Total Connections: %lu\n"
                 "Current Connections: %lu\n"
                 "Bytes Transferred: %lu\n"
                 "Errors: %lu\nEND\n",
                 metrics->total_connections,
                 metrics->current_connections,
                 metrics->bytes_transferred,
                 metrics->errors);
    }
}
