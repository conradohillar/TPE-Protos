#include <s5admin_parser.h>
#include <logger.h>

static void s5admin_parse_command_args(s5admin_cmd_parsed_t* parsed, char * token, char ** save_ptr, int expected_args);

static s5admin_cmd_info_t _commands[] = {
    {"ADD_USER", 2},
    {"REMOVE_USER", 1},
    {"LIST_USERS", 0},
    {"GET_METRICS", 0},
    {"GET_ACCESS_REGISTER", 0},
    {"SET_LOGLEVEL", 1},
    {"SET_MAX_CONN", 1},
    {"SET_BUFF", 1},
    {"GET_CONFIG", 0},
    {"HELP", 0},
    {"PING", 0},
    {"EXIT", 0},
};

void s5admin_parse_command(s5admin_cmd_parsed_t * parsed, char * cmd) {
    parsed->cmd = CMD_INVALID;
    strcpy(parsed->arg1, "invalid command");

    char* saveptr;
    char* token = strtok_r(cmd, " \r\n", &saveptr);

    for(unsigned int i = 0; i < sizeof(_commands) / sizeof(_commands[0]); i++) {
        if (strcmp(token, _commands[i].name) == 0) {
            parsed->cmd = i; 
            s5admin_parse_command_args(parsed, token, &saveptr, _commands[i].expected_args);
            return;
        }
    }
    parsed->cmd = CMD_INVALID;
}

static void s5admin_parse_command_args(s5admin_cmd_parsed_t* parsed, char * token, char ** save_ptr, int expected_args) {
    switch(expected_args){
        case 0:
            if (strtok_r(NULL, " \r\n", save_ptr)) {
                goto too_many_args; 
            }
            return;

        case 1:
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token) {
                if (strlen(token) > ARGS_LEN) {
                    while (strtok_r(NULL, " \r\n", save_ptr)); // Consumir toda la línea
                    goto invalid_argument;
                }
                strncpy(parsed->arg1, token, ARGS_LEN);
            } else {
                goto missing_args;
            }
            if (strtok_r(NULL, " \r\n", save_ptr)) {
                goto too_many_args;
            }
            return;

        case 2:
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token) {
                if (strlen(token) > ARGS_LEN) {
                    while (strtok_r(NULL, " \r\n", save_ptr));
                    goto invalid_argument;
                }
                strncpy(parsed->arg1, token, ARGS_LEN);
            } else {
                goto missing_args;
            }
            token = strtok_r(NULL, " \r\n", save_ptr);
            if (token) {
                if (strlen(token) > ARGS_LEN) {
                    while (strtok_r(NULL, " \r\n", save_ptr));
                    goto invalid_argument;
                }
                strncpy(parsed->arg2, token, ARGS_LEN);
            } else {
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

    invalid_argument:
        parsed->cmd = CMD_INVALID; 
        strcpy(parsed->arg1, "argument too long");
        return;
}
