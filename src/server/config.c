#include <config.h>

void config_process_command(config_cmd_parsed_t *parsed_cmd, char *response, size_t response_size, access_register_t *access_register) {
    //TODO: Implementar la lógica de cada comando
    switch (parsed_cmd->cmd) {
        case CMD_HELP:
            snprintf(response, response_size,
                "ADD_USER <usuario> <password>\nREMOVE_USER <usuario>\nLIST_USERS\nGET_METRICS\nGET_ACCESS_REGISTER\nSET_TIMEOUT <segundos>\nSET_BUFF <bytes>\nGET_CONFIG\nHELP\nPING\nEXIT\nEND\n");
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
        case CMD_GET_ACCESS_REGISTER:
            access_register_print(access_register, response, response_size);
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
        case CMD_EXIT:
            snprintf(response, response_size, "BYE\n");
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

    } else if (strcmp(token, "GET_ACCESS_REGISTER") == 0) {
        parsed->cmd = CMD_GET_ACCESS_REGISTER;
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

    } else if (strcmp(token, "EXIT") == 0) {
        parsed->cmd = CMD_EXIT;
        token = strtok_r(NULL, " \r\n", &saveptr);
        if (token) {parsed->cmd = CMD_INVALID;} // Demasiados argumentos

    } else {
        parsed->cmd = CMD_INVALID; // Comando desconocido
    }

    return parsed;
}

int config_handler(const char *cmd, char *response, size_t response_size, access_register_t *access_register) {
    config_cmd_parsed_t *parsed = config_parse_command(cmd);
    if (!parsed) {
        fprintf(stderr, "Error de parseo\n");
        return -1;
    }
    config_process_command(parsed, response, response_size, access_register);
    free(parsed);
    return strlen(response);
}