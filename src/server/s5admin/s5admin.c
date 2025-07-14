#include <s5admin.h>

typedef struct {
    int fd;
    buffer in_buff;
    buffer out_buff;
    uint8_t in_data[MAX_CMD_LEN];
    uint8_t out_data[MAX_RESPONSE_LEN];

} admin_conn_t;

static void admin_read(struct selector_key* key);
static void admin_write(struct selector_key* key);
static void admin_close(struct selector_key* key);

static const fd_handler admin_handler = {
    .handle_read = admin_read,
    .handle_write = admin_write,
    .handle_close = admin_close
};

void s5admin_passive_accept(struct selector_key* key) {
    admin_conn_t* conn = calloc(1, sizeof(admin_conn_t));
    if (conn == NULL) {
        LOG_MSG(ERROR, "Error allocating memory for admin connection");
        return;
    }
    int fd = passive_accept(key, conn, &admin_handler);
    if (fd < 0) {
        LOG_MSG(ERROR, "Failed to accept admin connection");
        free(conn);
        return;
    }
    conn->fd = fd;
    LOG(INFO, "Admin client connected on fd %d", fd);

    buffer_init(&conn->in_buff, MAX_CMD_LEN, conn->in_data);
    buffer_init(&conn->out_buff, MAX_RESPONSE_LEN, conn->out_data);
}

static void admin_read(struct selector_key* key) {
    admin_conn_t* conn = key->data;

    if (!buffer_can_write(&conn->in_buff)) { // Buffer de entrada lleno
        LOG(DEBUG, "Input buffer full for fd %d", key->fd);
        // NO DEBERÍA PASAR
        return;
    }

    size_t n;
    uint8_t* write_ptr = buffer_write_ptr(&conn->in_buff, &n);

    ssize_t n_read = recv(key->fd, write_ptr, n, MSG_DONTWAIT);
    if (n_read <= 0) {
        LOG(INFO, "Admin connection closed on fd %d", conn->fd);
        selector_unregister_fd(key->s, conn->fd);
        return;
    }
    LOG(DEBUG, "Read %zd bytes from admin connection fd %d", n_read, conn->fd);
    buffer_write_adv(&conn->in_buff, n_read);

    size_t n_parse;
    char* parse_ptr = (char*)buffer_read_ptr(&conn->in_buff, &n_parse);
    char* newline = strchr((const char*)parse_ptr, '\n');
    if (newline) {
        *newline = '\0';
        LOG(DEBUG, "Processing admin command: %s", parse_ptr);
        if (!buffer_can_write(&conn->out_buff)) {
            LOG(WARNING, "Output buffer full for fd %d", key->fd);
            // NO DEBERÍA PASAR
            return;
        }
        size_t n_response;
        char* response_ptr = (char*)buffer_write_ptr(&conn->out_buff, &n_response);
        int config_result;
        if(parse_ptr[0] == '\0'){ // Comando vacío
            strcpy(response_ptr, parse_ptr);
            config_result = 1;
        } else {
            config_result = config_handler(parse_ptr, response_ptr, n_response);
            if (config_result < 0) {
                selector_unregister_fd(key->s, conn->fd);
                close(conn->fd);
                free(conn);
                return;
            }
        }
        // Comando leído del in_buff
        size_t cmd_len = (newline - parse_ptr) + 1;
        buffer_read_adv(&conn->in_buff, cmd_len);
        // Respuesta generada y escrita en out_buff
        buffer_write_adv(&conn->out_buff, config_result);

        selector_set_interest_key(key, OP_WRITE);
    }
}

static void admin_write(struct selector_key* key) {
    admin_conn_t* conn = key->data;
    if (!buffer_can_read(&conn->out_buff)) {
        LOG(WARNING, "Output buffer empty for fd %d", key->fd);
        return;
    }

    size_t n;
    uint8_t* read_ptr = buffer_read_ptr(&conn->out_buff, &n);

    ssize_t n_written = send(key->fd, read_ptr, n, MSG_DONTWAIT);
    if (n_written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        LOG(ERROR, "Error writing to admin connection fd %d: %s", conn->fd, strerror(errno));
        selector_unregister_fd(key->s, conn->fd);
        close(conn->fd);
        free(conn);
        return;
    }
    LOG(DEBUG, "Wrote %zd bytes to admin connection fd %d", n_written, conn->fd);
    buffer_read_adv(&conn->out_buff, n_written);
    buffer_compact(&conn->out_buff);

    selector_set_interest_key(key, OP_READ);
}

static void admin_close(struct selector_key* key) {
    admin_conn_t* conn = key->data;
    LOG(INFO, "Closing admin connection on fd %d", conn->fd);
    close(conn->fd);
    free(conn);
}
