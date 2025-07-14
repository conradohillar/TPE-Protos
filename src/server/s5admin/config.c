#include <config.h>
#include <logger.h>

static s5admin_cmd_parsed_t _parsed_cmd;

static void handle_add_user(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_remove_user(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_list_users(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_metrics(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_access_register(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_loglevel(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_max_conn(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_set_buff(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_get_config(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_help(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_ping(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);
static void handle_exit(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size);

static s5admin_cmd_handler_t _command_handlers[] = {
    handle_add_user,
    handle_remove_user,
    handle_list_users,
    handle_get_metrics,
    handle_get_access_register,
    handle_set_loglevel,
    handle_set_max_conn,
    handle_set_buff,
    handle_get_config,
    handle_help,
    handle_ping,
    handle_exit
};

int config_handler(char* cmd, char* response, size_t response_size) {
    s5admin_parse_command(&_parsed_cmd, cmd);
    if(_parsed_cmd.cmd == CMD_INVALID){
        LOG_MSG(WARNING, "Invalid command received");
        snprintf(response, response_size, "ERROR: %s\n", _parsed_cmd.arg1);
    } else {
        s5admin_cmd_handler_t handler = _command_handlers[_parsed_cmd.cmd];
        handler(&_parsed_cmd, response, response_size);
    }
    return (int)strlen(response);
}

static void handle_add_user(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "ADD_USER command executed");
    if (auth_add_user(parsed_cmd->arg1, parsed_cmd->arg2)) {
        LOG(INFO, "User added successfully: %s", parsed_cmd->arg1);
        snprintf(response, response_size, "OK\n");
    } else {
        LOG(WARNING, "Failed to add user (already exists): %s", parsed_cmd->arg1);
        snprintf(response, response_size, "ERROR: %s already exists\n", parsed_cmd->arg1);
    }
}

static void handle_remove_user(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "REMOVE_USER command executed");
    if (auth_remove_user(parsed_cmd->arg1)) {
        LOG(INFO, "User removed successfully: %s", parsed_cmd->arg1);
        snprintf(response, response_size, "OK\n");
    } else {
        LOG(WARNING, "Failed to remove user (does not exist): %s", parsed_cmd->arg1);
        snprintf(response, response_size, "ERROR: %s not found\n", parsed_cmd->arg1);
    }
}

static void handle_list_users(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "LIST_USERS command executed");
    auth_list_users(response, response_size);
}

static void handle_get_metrics(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_METRICS command executed");
    metrics_print(get_server_data()->metrics, response, response_size);
}

static void handle_get_access_register(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_ACCESS_REGISTER command executed");
    access_register_print(get_server_data()->access_register, response, response_size);
}

static void handle_set_loglevel(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
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

static void handle_set_max_conn(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "SET_MAX_CONN command executed");
    if (set_max_conn(atoi(parsed_cmd->arg1)) == 0) {
        snprintf(response, response_size, "OK\n");
    } else {
        snprintf(response, response_size, "ERROR: max connections must be at least 1\n");
    }
}

static void handle_set_buff(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "SET_BUFF command executed");
    if (set_buffer_size(atoi(parsed_cmd->arg1)) == 0) {
        snprintf(response, response_size, "OK\n");
    } else {
        snprintf(response, response_size, "ERROR: buffer size must be at least %d bytes\n", BUFF_MIN_LEN);
    }
}

static void handle_get_config(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "GET_CONFIG command executed");
    server_data_t* server_data = get_server_data();
    snprintf(response, response_size, "LOG_LEVEL: %s\nBUFFER_SIZE: %dB\nMAX_CONN: %d\nEND\n",
        get_log_level_string(), server_data->buffer_size, server_data->max_conn);
}

static void handle_help(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "HELP command executed");
    snprintf(
        response,
        response_size,
        "ADD_USER <usuario> <password>\nREMOVE_USER "
        "<usuario>\nLIST_USERS\nGET_METRICS\nGET_ACCESS_REGISTER\nSET_LOGLEVEL "
        "<DEBUG|INFO|WARNING|ERROR>\nSET_MAX_CONN <cantidad>\nSET_BUFF <bytes>\nGET_CONFIG\n"
        "HELP\nPING\nEXIT\nEND\n");
}

static void handle_ping(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "PING command executed");
    snprintf(response, response_size, "PONG\n");
}

static void handle_exit(s5admin_cmd_parsed_t* parsed_cmd, char* response, size_t response_size){
    LOG_MSG(DEBUG, "EXIT command executed");
    snprintf(response, response_size, "BYE\n");
}
