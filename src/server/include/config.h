#ifndef CONFIG_H
#define CONFIG_H

#include <access_register.h>
#include <server_data.h>
#include <auth_table.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>
#include <stdlib.h>

#define ARGS_LEN 256
#define MAX_CMD_LEN 512
#define MAX_RESPONSE_LEN 1024 * 100

#define DEFAULT_CONFIG_HOST "127.0.0.1"
#define DEFAULT_CONFIG_PORT 8080

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
} config_command_t;

// Estructura para el resultado del parseo
typedef struct {
    config_command_t cmd;
    char arg1[ARGS_LEN];
    char arg2[ARGS_LEN];
} config_cmd_parsed_t;

// Estructura para almacenar información de los comandos
typedef void (*config_cmd_handler_t)(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

typedef struct {
    const char* name;
    int expected_args;
    config_cmd_handler_t handler;
} config_cmd_info_t;


/*
 * Funciones para manejar comandos de configuración desde el cliente administrador
 * Manejo de parámetros globales como timeout, buffer size, etc.
 */

/**
 * Maneja un comando de configuración recibido del cliente administrador.
 * @param sockfd El socket del cliente administrador.
 * @param cmd El comando recibido (ej. "SET timeout 30").
 * @param response El buffer donde se escribirá la respuesta.
 * @param response_size El tamaño máximo del buffer de respuesta.
 * @return El tamaño de la respuesta escrita en el buffer, o -1 si hubo error de parseo
 */
int config_handler(char* cmd, char* response, size_t response_size);

/**
 * Ejecuta la acción correspondiente al comando de configuración y devuelve la respuesta correspondiente.
 * @param cmd El comando a procesar (ej. "SET timeout 30").
 * @param response El buffer donde se escribirá la respuesta.
 * @param response_size El tamaño máximo del buffer de respuesta.
 * @return 0 en caso de éxito, o un código de error negativo.
 */
void config_process_command(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

/**
 * Parsea un comando de configuración recibido del cliente administrador.
 * @param cmd El comando a parsear (ej. "SET timeout 30").
 * @return Un puntero a una estructura config_cmd_parsed_t con el resultado del parseo,
 *         o NULL en caso de error.
 */
config_cmd_parsed_t* config_parse_command(char* cmd);

#endif // CONFIG_H
