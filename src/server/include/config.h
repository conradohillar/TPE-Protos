#ifndef CONFIG_H
#define CONFIG_H

#include <access_register.h>
#include <server_data.h>
#include <auth_table.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define ARGS_LEN 256
#define MAX_CMD_LEN 512
#define MAX_RESPONSE_LEN 1024 * 100

#define DEFAULT_CONFIG_HOST "127.0.0.1"
#define DEFAULT_CONFIG_PORT 9090

// Enum de comandos posibles
typedef enum {
    CMD_ADD_USER,
    CMD_REMOVE_USER,
    CMD_LIST_USERS,
    CMD_GET_METRICS,
    CMD_GET_ACCESS_REGISTER,
    CMD_SET_TIMEOUT,
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
int config_handler(const char *cmd, char *response, size_t response_size);

/**
 * Ejecuta la acción correspondiente al comando de configuración y devuelve la respuesta correspondiente.
 * @param cmd El comando a procesar (ej. "SET timeout 30").
 * @param response El buffer donde se escribirá la respuesta.
 * @param response_size El tamaño máximo del buffer de respuesta.
 * @return 0 en caso de éxito, o un código de error negativo.
 */
void config_process_command(config_cmd_parsed_t *parsed_cmd, char *response, size_t response_size);

/**
 * Parsea un comando de configuración recibido del cliente administrador.
 * @param cmd El comando a parsear (ej. "SET timeout 30").
 * @return Un puntero a una estructura config_cmd_parsed_t con el resultado del parseo,
 *         o NULL en caso de error.
 */
config_cmd_parsed_t *config_parse_command(const char *cmd);

// Setters y getters para parámetros globales
int config_set_timeout(int seconds);
int config_set_buffer_size(size_t bytes);
int config_get_timeout(void);
size_t config_get_buffer_size(void);

// Devuelve el estado/configuración actual en formato texto
void config_get_status(char *out, size_t outlen);

#endif // CONFIG_H
