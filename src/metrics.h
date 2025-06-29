#ifndef METRICS_H
#define METRICS_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    uint64_t total_connections;
    uint64_t current_connections;
    uint64_t bytes_transferred;
    uint64_t errors;
    // Agregar más métricas según necesidad
} server_metrics_t;

// Inicializa las métricas
server_metrics_t *metrics_init();

// Setters y updaters
void metrics_inc_total_connections(server_metrics_t *metrics);
void metrics_inc_current_connections(server_metrics_t *metrics);
void metrics_dec_current_connections(server_metrics_t *metrics);
void metrics_add_bytes(server_metrics_t *metrics, uint64_t bytes);
void metrics_inc_errors(server_metrics_t *metrics);

#endif // METRICS_H
