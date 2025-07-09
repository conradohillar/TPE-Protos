#include "logger.h"
#include <config.h>

void config_process_command(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size) {
    // TODO: Implementar la lógica de cada comando
    LOG(DEBUG, "Processing admin command: %d", parsed_cmd->cmd);
    switch (parsed_cmd->cmd) {
    case CMD_HELP:
        LOG_MSG(DEBUG, "Help command executed");
        snprintf(
            response,
            response_size,
            "ADD_USER <usuario> <password>\nREMOVE_USER "
            "<usuario>\nLIST_USERS\nGET_METRICS\nGET_ACCESS_REGISTER\nSET_TIMEOUT "
            "<segundos>\nSET_BUFF <bytes>\nGET_CONFIG\nHELP\nPING\nEXIT\nEND\n");
        break;
    case CMD_PING:
        LOG_MSG(DEBUG, "Ping command executed");
        snprintf(response, response_size, "PONG\n");
        break;
    case CMD_ADD_USER:
        if (auth_add_user(parsed_cmd->arg1, parsed_cmd->arg2)) {
            LOG(INFO, "User added successfully: %s", parsed_cmd->arg1);
            snprintf(response, response_size, "OK\n");
        } else {
            LOG(WARNING, "Failed to add user (already exists): %s", parsed_cmd->arg1);
            snprintf(response, response_size, "ERROR: %s already exists\n", parsed_cmd->arg1);
        }
        break;
    case CMD_REMOVE_USER:
        if (auth_remove_user(parsed_cmd->arg1)) {
            LOG(INFO, "User removed successfully: %s", parsed_cmd->arg1);
            snprintf(response, response_size, "OK\n");
        } else {
            LOG(WARNING, "Failed to remove user (does not exist): %s", parsed_cmd->arg1);
            snprintf(response, response_size, "ERROR: %s does not exist\n", parsed_cmd->arg1);
        }
        break;
    case CMD_LIST_USERS:
        LOG_MSG(DEBUG, "List users command executed");
        auth_list_users(response, response_size);
        break;
    case CMD_GET_METRICS:
        LOG_MSG(DEBUG, "Get metrics command executed");
        metrics_print(get_server_data()->metrics, response, response_size);
        break;
    case CMD_GET_ACCESS_REGISTER:
        LOG_MSG(DEBUG, "Get access register command executed");
        access_register_print(get_server_data()->access_register, response, response_size);
        break;
    case CMD_SET_TIMEOUT:
        if (set_timeout(atoi(parsed_cmd->arg1)) == 0) {
            snprintf(response, response_size, "OK\n");
        } else {
            snprintf(response, response_size, "ERROR: Timeout must be at least %d seconds\n", TIMEOUT_MIN);
        }
        break;
    case CMD_SET_BUFF:
        if (set_buffer_size(atoi(parsed_cmd->arg1)) == 0) {
            snprintf(response, response_size, "OK\n");
        } else {
            snprintf(response, response_size, "ERROR: Buffer size must be at least %d bytes\n", BUFF_MIN_LEN);
        }
        break;
    case CMD_GET_CONFIG:
        LOG_MSG(DEBUG, "Get config command executed");
        server_data_t* server_data = get_server_data();
        snprintf(response, response_size, "Configuración actual: timeout=%ds, buffer_size=%dB\nOK\n", server_data->timeout, server_data->buffer_size);
        break;
    case CMD_EXIT:
        LOG_MSG(DEBUG, "Exit command executed");
        snprintf(response, response_size, "BYE\n");
        break;
    case CMD_INVALID:
        LOG_MSG(WARNING, "Invalid command received");
        snprintf(response, response_size, "ERROR: invalid command\n");
        break;
    default:
        LOG(ERROR, "Unknown command type: %d", parsed_cmd->cmd);
        break;
    }
}

config_cmd_parsed_t* config_parse_command(const char* cmd) {
    config_cmd_parsed_t* parsed = malloc(sizeof(config_cmd_parsed_t));
    if (!parsed)
        return NULL;

    // Copiar el comando a un buffer local para no modificar el original
    char local_cmd[MAX_CMD_LEN];
    strncpy(local_cmd, cmd, sizeof(local_cmd) - 1);
    local_cmd[sizeof(local_cmd) - 1] = '\0';

    // Inicializar el comando como inválido
    parsed->cmd = CMD_INVALID;
    memset(parsed->arg1, 0, sizeof(parsed->arg1));
    memset(parsed->arg2, 0, sizeof(parsed->arg2));

    // Parsear el comando (manejo robusto para comandos sin argumentos)
    char* saveptr;
    char* token = strtok_r(local_cmd, " \r\n", &saveptr);
    if (!token) {
        free(parsed);
        return NULL; // Comando vacío
    }

    if (strcmp(token, "ADD_USER") == 0) {
        parsed->cmd = CMD_ADD_USER;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token)
            strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {
            parsed->cmd = CMD_INVALID;
        } // Falta el usuario
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token)
            strncpy(parsed->arg2, token, sizeof(parsed->arg2) - 1);
        else {
            parsed->cmd = CMD_INVALID;
        } // Falta la contraseña
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "REMOVE_USER") == 0) {
        parsed->cmd = CMD_REMOVE_USER;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token)
            strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {
            parsed->cmd = CMD_INVALID;
        } // Falta el usuario
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "LIST_USERS") == 0) {
        parsed->cmd = CMD_LIST_USERS;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "GET_METRICS") == 0) {
        parsed->cmd = CMD_GET_METRICS;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "GET_ACCESS_REGISTER") == 0) {
        parsed->cmd = CMD_GET_ACCESS_REGISTER;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "SET_TIMEOUT") == 0) {
        parsed->cmd = CMD_SET_TIMEOUT;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token)
            strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {
            parsed->cmd = CMD_INVALID;
        } // Falta el valor del timeout
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "SET_BUFF") == 0) {
        parsed->cmd = CMD_SET_BUFF;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token)
            strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {
            parsed->cmd = CMD_INVALID;
        } // Falta el valor del buffer
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "GET_CONFIG") == 0) {
        parsed->cmd = CMD_GET_CONFIG;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "HELP") == 0) {
        parsed->cmd = CMD_HELP;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "PING") == 0) {
        parsed->cmd = CMD_PING;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else if (strcmp(token, "EXIT") == 0) {
        parsed->cmd = CMD_EXIT;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {
            parsed->cmd = CMD_INVALID;
        } // Demasiados argumentos

    } else {
        parsed->cmd = CMD_INVALID; // Comando desconocido
    }

    return parsed;
}

int config_handler(const char* cmd, char* response, size_t response_size) {
    config_cmd_parsed_t* parsed = config_parse_command(cmd);
    if (!parsed) {
        LOG_MSG(ERROR, "Error de parseo");
        return -1;
    }
    config_process_command(parsed, response, response_size);
    free(parsed);
    return strlen(response);
}