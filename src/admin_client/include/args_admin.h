#ifndef ARGS_ADMIN_H_
#define ARGS_ADMIN_H_

#include <logger.h>

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
