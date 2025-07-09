#include <server_data.h>

static server_data_t* _server_data = NULL;

void server_data_init() {
    _server_data = malloc(sizeof(server_data_t));
    if (_server_data == NULL) {
        LOG_MSG(ERROR, "Failed to allocate memory for server data");
        exit(EXIT_FAILURE);
    }

    LOG_MSG(DEBUG, "Server data allocated successfully");
    _server_data->metrics = metrics_init();
    _server_data->access_register = access_register_init();
    _server_data->buffer_size = BUFF_DEFAULT_LEN; // Default buffer size
    _server_data->timeout = TIMEOUT_DEFAULT;
}

server_data_t* get_server_data() { return _server_data; }

void server_data_destroy() {
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

int set_timeout(int seconds) {
    if (seconds < TIMEOUT_MIN) {
        LOG(ERROR, "Admin tried to set timeout below minimum: %d seconds", seconds);
        return -1;
    }
    _server_data->timeout = seconds;
    LOG(INFO, "Timeout set to %d seconds", seconds);
    return 0;
}
