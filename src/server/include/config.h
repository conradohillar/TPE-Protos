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


typedef void (*s5admin_cmd_handler_t)(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

int config_handler(char* cmd, char* response, size_t response_size);

s5admin_cmd_parsed_t* config_parse_command(char* cmd);

#endif // CONFIG_H
