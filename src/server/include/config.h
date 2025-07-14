#ifndef CONFIG_H
#define CONFIG_H

#include <s5admin_parser.h>
#include <access_register.h>
#include <server_data.h>
#include <auth_table.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <logger.h>
#include <stdlib.h>

#define DEFAULT_CONFIG_HOST "127.0.0.1"
#define DEFAULT_CONFIG_PORT 8080

// Handlers de comandos
typedef void (*s5admin_cmd_handler_t)(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

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
 * Parsea un comando de configuración recibido del cliente administrador.
 * @param cmd El comando a parsear (ej. "SET timeout 30").
 * @return Un puntero a una estructura config_cmd_parsed_t con el resultado del parseo,
 *         o NULL en caso de error.
 */
s5admin_cmd_parsed_t* config_parse_command(char* cmd);

#endif // CONFIG_H
