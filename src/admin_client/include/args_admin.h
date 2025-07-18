#ifndef ARGS_ADMIN_H_
#define ARGS_ADMIN_H_

#include <logger.h>

#define DEFAULT_SERVER_HOST "127.0.0.1"
#define DEFAULT_SERVER_PORT 8080

typedef struct admin_args {
    char* host;
    int port;

    char* log_file;

    log_level_t log_level;    
} admin_args;

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc, char** argv, struct admin_args* args);

#endif
