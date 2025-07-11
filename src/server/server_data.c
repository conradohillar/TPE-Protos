#include <server_data.h>

static server_data_t* _server_data = NULL;

void server_data_init(void) {
    _server_data = malloc(sizeof(server_data_t));
    if (_server_data == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for server data");
        exit(EXIT_FAILURE);
    }

    LOG_MSG(DEBUG, "Server data allocated successfully");
    _server_data->metrics = metrics_init();
    _server_data->access_register = access_register_init();
    _server_data->buffer_size = BUFF_DEFAULT_LEN; 
    _server_data->max_conn = MAX_CONN_DEFAULT;
}

server_data_t* get_server_data(void) { return _server_data; }

void server_data_destroy(void) {
    if (_server_data != NULL) {
        LOG_MSG(DEBUG, "Freeing server data");
        free(_server_data->access_register);
        free(_server_data->metrics);
        free(_server_data);
        _server_data = NULL;
    } else {
        LOG_MSG(WARNING, "Attempted to free a NULL server data");
    }
}

int set_buffer_size(int size) {
    if (size < BUFF_MIN_LEN) {
        LOG(ERROR, "Admin tried to set buffer size below minimum: %d bytes", size);
        return -1;
    }
    _server_data->buffer_size = size;
    LOG(INFO, "Buffer size set to %d bytes", size);
    return 0;
}

int set_max_conn(int max_conn) {
    if (max_conn < 1) {
        LOG(ERROR, "Admin tried to set max connections below minimum: %d", max_conn);
        return -1;
    }
    _server_data->max_conn = max_conn;
    LOG(INFO, "Max connections set to %d", max_conn);
    return 0;
}
