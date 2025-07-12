#include <access_register.h>

access_register_t* access_register_init(void) {
    access_register_t* reg = malloc(sizeof(access_register_t));
    if (!reg) {
        LOG_MSG(ERROR, "Error initializing access register");
        return NULL;
    }
    reg->head = 0;
    reg->tail = 0;
    reg->count = 0;
    LOG_MSG(INFO, "Access register initialized");
    return reg;
}

void access_register_add_entry(access_register_t* reg, const char* username, const char* src_ip, uint16_t src_port, const char* destination, uint16_t dest_port, int status_code, time_t timestamp) {
    if (!reg)
        return;
    access_entry_t* entry = &reg->entries[reg->tail];
    struct tm tm_info;
    gmtime_r(&timestamp, &tm_info);
    strftime(entry->iso_date, sizeof(entry->iso_date), "%Y-%m-%dT%H:%M:%SZ", &tm_info);
    strncpy(entry->username, username, MAX_USERNAME_LEN - 1);
    entry->username[MAX_USERNAME_LEN - 1] = '\0';
    entry->register_type = DEFAULT_REG_TYPE;
    strncpy(entry->src_ip, src_ip, IP_MAX_LEN - 1);
    entry->src_ip[IP_MAX_LEN - 1] = '\0';
    entry->src_port = src_port;
    strncpy(entry->destination, destination, DEST_MAX_LEN - 1);
    entry->destination[DEST_MAX_LEN - 1] = '\0';
    entry->dest_port = dest_port;
    entry->status_code = status_code;
    reg->tail = (reg->tail + 1) % REGISTER_MAX_ENTRIES;
    if (reg->count < REGISTER_MAX_ENTRIES) {
        reg->count++;
    } else {
        reg->head = (reg->head + 1) % REGISTER_MAX_ENTRIES; // sobrescribe el más antiguo
    }
    LOG(DEBUG, "Access entry added: user=%s, src=%s:%u, dest=%s:%u, status=%d", username, src_ip, src_port, destination, dest_port, status_code);
}

size_t access_register_print(const access_register_t* reg, char* response, size_t response_size) {
    if (!reg)
        return 0;
    size_t idx = reg->head;
    size_t written = 0;
    int n = snprintf(response + written, response_size - written,
        "%-25s %-15s %-10s %-15s %-12s %-20s %-12s %-10s\n",
        "Fecha", "Usuario", "Tipo", "IP_origen", "Pto_org", "Destino", "Pto_dst", "Status");
    if (n < 0 || (size_t)n >= response_size - written)
        return written;
    written += n;
    for (size_t i = 0; i < reg->count; i++) {
        const access_entry_t* entry = &reg->entries[idx];
        n = snprintf(response + written, response_size - written,
            "%-25s %-15s %-10c %-15s %-12d %-20s %-12d %-10d\n",
            entry->iso_date,
            entry->username,
            entry->register_type,
            entry->src_ip,
            entry->src_port,
            entry->destination,
            entry->dest_port,
            entry->status_code);
        if (n < 0 || (size_t)n >= response_size - written)
            break;
        written += n;
        idx = (idx + 1) % REGISTER_MAX_ENTRIES;
    }
    if (written < response_size) {
        n = snprintf(response + written, response_size - written, "END\n");
        if (n >= 0 && (size_t)n < response_size - written) {
            written += n;
        }
    }
    return written;
}
