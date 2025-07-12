#include <logger.h>
#include <config.h>

static void check_command_args(config_cmd_parsed_t* parsed, char * token, char ** save_ptr, int expected_args);

static void handle_add_user(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_remove_user(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_list_users(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_metrics(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_access_register(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_loglevel(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_max_conn(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_buff(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_config(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_help(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_ping(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_exit(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

static config_cmd_info_t _commands[] = {
    {"ADD_USER", 2, handle_add_user},
    {"REMOVE_USER", 1, handle_remove_user},
    {"LIST_USERS", 0, handle_list_users},
    {"GET_METRICS", 0, handle_get_metrics},
    {"GET_ACCESS_REGISTER", 0, handle_get_access_register},
    {"SET_LOGLEVEL", 1, handle_set_loglevel},
    {"SET_MAX_CONN", 1, handle_set_max_conn},
    {"SET_BUFF", 1, handle_set_buff},
    {"GET_CONFIG", 0, handle_get_config},
    {"HELP", 0, handle_help},
    {"PING", 0, handle_ping},
    {"EXIT", 0, handle_exit},
};

int config_handler(char* cmd, char* response, size_t response_size) {
    config_cmd_parsed_t* parsed = config_parse_command(cmd);
    if (!parsed) {
        return -1;
    }
    
    if(parsed->cmd == CMD_INVALID){
        LOG_MSG(WARNING, "Invalid command received");
        snprintf(response, response_size, "ERROR: %s\n", parsed->arg1);
    } else {
        _commands[parsed->cmd].handler(parsed, response, response_size);
    }
    free(parsed);
    return (int)strlen(response);
}

config_cmd_parsed_t* config_parse_command(char* cmd) {
    config_cmd_parsed_t* parsed = malloc(sizeof(config_cmd_parsed_t));
    if (!parsed)
        return NULL;

    parsed->cmd = CMD_INVALID;
    strcpy(parsed->arg1, "invalid command");

    // Parsear el comando
    char* saveptr;
    char* token = strtok_r(cmd, " \r\n", &saveptr);

    for(unsigned int i = 0; i < sizeof(_commands) / sizeof(_commands[0]); i++) {
        if (strcmp(token, _commands[i].name) == 0) {
            parsed->cmd = i; 
            check_command_args(parsed, token, &saveptr, _commands[i].expected_args);
            return parsed;
        }
    }
    parsed->cmd = CMD_INVALID;
    return parsed;
}

static void check_command_args(config_cmd_parsed_t* parsed, char * token, char ** save_ptr, int expected_args) {
    switch(expected_args){
        case 0:
            if (strtok_r(NULL, " \r\n", save_ptr)) {
                goto too_many_args; 
            }
            return;

        case 1:
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token)
                strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
            else {
                goto missing_args;
            }
            if (strtok_r(NULL, " \r\n", save_ptr)) {
                goto too_many_args;
            }
            return;

        case 2:
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token)
                strncpy(parsed->arg1, token, sizeof(parsed->arg1) - 1);
            else {
                goto missing_args;
            }
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token)
                strncpy(parsed->arg2, token, sizeof(parsed->arg2) - 1);
            else {
                goto missing_args;
            }
            if (strtok_r(NULL, " \r\n", save_ptr)) {
                goto too_many_args;
            }
            return;

        default:
            LOG(WARNING, "Unsupported number of expected arguments: %d", expected_args);
            parsed->cmd = CMD_INVALID;
            return;
    }

    missing_args:
        parsed->cmd = CMD_INVALID; 
        strcpy(parsed->arg1, "missing argument");
        return;

    too_many_args:
        parsed->cmd = CMD_INVALID; 
        strcpy(parsed->arg1, "too many arguments");
        return;   
}

static void handle_add_user(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "ADD_USER command executed");
    if (auth_add_user(parsed_cmd->arg1, parsed_cmd->arg2)) {
        LOG(INFO, "User added successfully: %s", parsed_cmd->arg1);
        snprintf(response, response_size, "OK\n");
    } else {
        LOG(WARNING, "Failed to add user (already exists): %s", parsed_cmd->arg1);
        snprintf(response, response_size, "ERROR: %s already exists\n", parsed_cmd->arg1);
    }
}

static void handle_remove_user(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "REMOVE_USER command executed");
    if (auth_remove_user(parsed_cmd->arg1)) {
        LOG(INFO, "User removed successfully: %s", parsed_cmd->arg1);
        snprintf(response, response_size, "OK\n");
    } else {
        LOG(WARNING, "Failed to remove user (does not exist): %s", parsed_cmd->arg1);
        snprintf(response, response_size, "ERROR: %s does not exist\n", parsed_cmd->arg1);
    }
}

static void handle_list_users(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "LIST_USERS command executed");
    auth_list_users(response, response_size);
}

static void handle_get_metrics(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_METRICS command executed");
    metrics_print(get_server_data()->metrics, response, response_size);
}

static void handle_get_access_register(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_ACCESS_REGISTER command executed");
    access_register_print(get_server_data()->access_register, response, response_size);
}

static void handle_set_loglevel(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "SET_LOGLEVEL command executed");
    if (strcmp(parsed_cmd->arg1, "DEBUG") == 0) {
        set_log_level(DEBUG);
    } else if (strcmp(parsed_cmd->arg1, "INFO") == 0) {
        set_log_level(INFO);
    } else if (strcmp(parsed_cmd->arg1, "WARNING") == 0) {
        set_log_level(WARNING);
    } else if (strcmp(parsed_cmd->arg1, "ERROR") == 0) {
        set_log_level(ERROR);
    } else {
        snprintf(response, response_size, "ERROR: invalid log level\n");
        return;
    }
    snprintf(response, response_size, "OK\n");
}

static void handle_set_max_conn(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "SET_MAX_CONN command executed");
    if (set_max_conn(atoi(parsed_cmd->arg1)) == 0) {
        snprintf(response, response_size, "OK\n");
    } else {
        snprintf(response, response_size, "ERROR: max connections must be at least 1\n");
    }
}

static void handle_set_buff(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "SET_BUFF command executed");
    if (set_buffer_size(atoi(parsed_cmd->arg1)) == 0) {
        snprintf(response, response_size, "OK\n");
    } else {
        snprintf(response, response_size, "ERROR: buffer size must be at least %d bytes\n", BUFF_MIN_LEN);
    }
}

static void handle_get_config(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_CONFIG command executed");
    server_data_t* server_data = get_server_data();
    snprintf(response, response_size, "Configuración actual: log_level=%s, buffer_size=%dB, max_conn=%d\nOK\n", 
        get_log_level_string(), server_data->buffer_size, server_data->max_conn);
}

static void handle_help(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "HELP command executed");
    snprintf(
        response,
        response_size,
        "ADD_USER <usuario> <password>\nREMOVE_USER "
        "<usuario>\nLIST_USERS\nGET_METRICS\nGET_ACCESS_REGISTER\nSET_LOGLEVEL "
        "<DEBUG|INFO|WARNING|ERROR>\nSET_MAX_CONN <cantidad>\nSET_BUFF <bytes>\nGET_CONFIG\n"
        "HELP\nPING\nEXIT\nEND\n");
}

static void handle_ping(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "PING command executed");
    snprintf(response, response_size, "PONG\n");
}

static void handle_exit(config_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "EXIT command executed");
    snprintf(response, response_size, "BYE\n");
}
