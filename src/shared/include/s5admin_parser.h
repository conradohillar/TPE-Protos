#ifndef S5ADMIN_PARSER_H
#define S5ADMIN_PARSER_H

#include <string.h>
#include <stdlib.h>

#define ARGS_LEN 256
#define MAX_CMD_LEN 1024
#define MAX_RESPONSE_LEN 1024 * 100

// Enum de comandos posibles
typedef enum {
    CMD_ADD_USER,
    CMD_REMOVE_USER,
    CMD_LIST_USERS,
    CMD_GET_METRICS,
    CMD_GET_ACCESS_REGISTER,
    CMD_SET_LOGLEVEL,
    CMD_SET_MAX_CONN,
    CMD_SET_BUFF,
    CMD_GET_CONFIG,
    CMD_HELP,
    CMD_PING,
    CMD_EXIT,
    CMD_INVALID
} s5admin_command_t;

// Estructura para el resultado del parseo
typedef struct {
    s5admin_command_t cmd;
    char arg1[ARGS_LEN];
    char arg2[ARGS_LEN];
} s5admin_cmd_parsed_t;

// Estructura para almacenar información de los comandos
typedef struct {
    const char* name;
    int expected_args;
} s5admin_cmd_info_t;


void s5admin_parse_command(s5admin_cmd_parsed_t * parsed, char* cmd);

#endif // S5ADMIN_PARSER_H
