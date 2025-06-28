#include "config.h"

void config_process_command(config_cmd_parsed_t *parsed_cmd, char *response, size_t response_size) {
    //TODO: Implementar la lógica de cada comando
    switch (parsed_cmd->cmd) {
        case CMD_HELP:
            snprintf(response, response_size,
                "ADD_USER <usuario> <password>\nREMOVE_USER <usuario>\nLIST_USERS\nGET_METRICS\nGET_LOG\nSET_TIMEOUT <segundos>\nSET_BUFF <bytes>\nGET_CONFIG\nHELP\nPING\nEND\n");
            break;
        case CMD_PING:
            snprintf(response, response_size, "PONG\n");
            break;
        case CMD_ADD_USER:
            printf("Usuario %s agregado con contraseña %s\n", parsed_cmd->arg1, parsed_cmd->arg2);
            snprintf(response, response_size, "OK\n");
            break;
        case CMD_REMOVE_USER:
            printf("Usuario %s eliminado\n", parsed_cmd->arg1);
            snprintf(response, response_size, "OK\n");
            break;
        case CMD_LIST_USERS:
            snprintf(response, response_size, "user1\nuser2\nuser3\nEND\n");
            break;
        case CMD_GET_METRICS:
            snprintf(response, response_size, "Conexiones: 100, Bytes: 2048, Errores: 0\nOK\n");
            break;
        case CMD_GET_LOG:
            snprintf(response, response_size, "Log: [2023-10-01 12:00:00] Conexión aceptada desde 192.168.1.1\nOK\n");
            break;
        case CMD_SET_TIMEOUT:
            printf("Timeout configurado a %s segundos (simulado)\n", parsed_cmd->arg1);
            snprintf(response, response_size, "OK\n");
            break;
        case CMD_SET_BUFF:
            printf("Tamaño de buffer configurado a %s bytes (simulado)\n", parsed_cmd->arg1);
            snprintf(response, response_size, "OK\n");
            break;
        case CMD_GET_CONFIG:
            snprintf(response, response_size, "Configuración actual: timeout=30, buffer_size=1024\nOK\n");
            break;
        case CMD_INVALID:
            snprintf(response, response_size, "ERROR\n");
            break;
        default:
            break;
    }
}

config_cmd_parsed_t *config_parse_command(const char *cmd) {
    config_cmd_parsed_t *parsed = malloc(sizeof(config_cmd_parsed_t));
    if (!parsed) return NULL;

    // Copiar el comando a un buffer local para no modificar el original
    char local_cmd[MAX_CMD_LEN];
    strncpy(local_cmd, cmd, sizeof(local_cmd) - 1);
    local_cmd[sizeof(local_cmd) - 1] = '\0';

    // Inicializar el comando como inválido
    parsed->cmd = CMD_INVALID;
    memset(parsed->arg1, 0, sizeof(parsed->arg1));
    memset(parsed->arg2, 0, sizeof(parsed->arg2));

    // Parsear el comando (manejo robusto para comandos sin argumentos)
    char *saveptr;
    char *token = strtok_r(local_cmd, " \r\n", &saveptr);
    if (!token) {
        free(parsed);
        return NULL; // Comando vacío
    }

    if (strcmp(token, "ADD_USER") == 0) {
        parsed->cmd = CMD_ADD_USER;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {parsed->cmd = CMD_INVALID;} // Falta el usuario
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) strncpy(parsed->arg2, token, sizeof(parsed->arg2) - 1);
        else {parsed->cmd = CMD_INVALID;} // Falta la contraseña
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos
    
    } else if (strcmp(token, "REMOVE_USER") == 0) {
        parsed->cmd = CMD_REMOVE_USER;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {parsed->cmd = CMD_INVALID;} // Falta el usuario
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "LIST_USERS") == 0) {
        parsed->cmd = CMD_LIST_USERS;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "GET_METRICS") == 0) {
        parsed->cmd = CMD_GET_METRICS;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "GET_LOG") == 0) {
        parsed->cmd = CMD_GET_LOG;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "SET_TIMEOUT") == 0) {
        parsed->cmd = CMD_SET_TIMEOUT;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {parsed->cmd = CMD_INVALID;} // Falta el valor del timeout
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "SET_BUFF") == 0) {
        parsed->cmd = CMD_SET_BUFF;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
        else {parsed->cmd = CMD_INVALID;} // Falta el valor del buffer
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos
    
    } else if (strcmp(token, "GET_CONFIG") == 0) {
        parsed->cmd = CMD_GET_CONFIG;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else if (strcmp(token, "HELP") == 0) {
        parsed->cmd = CMD_HELP;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos
    
    } else if (strcmp(token, "PING") == 0) {
        parsed->cmd = CMD_PING;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else {
        parsed->cmd = CMD_INVALID; // Comando desconocido
    }

    return parsed;
}

void config_handler(int sockfd, const char *cmd) {
    uint8_t buf_data[MAX_RESPONSE_LEN];
    buffer buf;
    buffer_init(&buf, sizeof(buf_data), buf_data);
    char response[MAX_RESPONSE_LEN];
    
    config_cmd_parsed_t *parsed = config_parse_command(cmd);
    if (!parsed) {
        snprintf(response, sizeof(response), "Error en el parseo\n");
    } else {
        // Procesar el comando
        config_process_command(parsed, response, sizeof(response));
    }

    size_t len = strlen(response);
    memcpy(buf.data, response, len);
    buf.write = buf.data + len;
    sock_blocking_write(sockfd, &buf);
    free(parsed);
}