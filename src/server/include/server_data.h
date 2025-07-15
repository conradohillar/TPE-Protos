#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <access_register.h>
#include <metrics.h>
#include <defines.h>

typedef struct {
    // Metricas
    server_metrics_t* metrics;

    // Registro de accesos
    access_register_t* access_register;

    // Tamaño de los buffers de conexión
    unsigned int buffer_size;

    // Número máximo de conexiones
    unsigned int max_conn;

} server_data_t;

server_data_t* get_server_data(void);

void server_data_init(void);

void server_data_destroy(void);

int set_buffer_size(int size);

int set_max_conn(int max_conn);

#endif // SERVER_DATA_H
