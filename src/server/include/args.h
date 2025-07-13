#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include <logger.h>

#define MAX_USERS 10

struct users {
    char* name;
    char* pass;
};

typedef struct server_args {
    char* socks_addr;
    unsigned short socks_port;

    char* mng_addr;
    unsigned short mng_port;

    bool disectors_enabled;

    struct users users[MAX_USERS];
    unsigned short users_count;

    log_level_t log_level;  
    
    char* log_file;
} server_args;

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc, char** argv, struct server_args* args);

#endif
