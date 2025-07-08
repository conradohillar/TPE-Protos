#include "access_register.h"
#include "logger.h"

access_register_t *access_register_init() {
  access_register_t *reg = malloc(sizeof(access_register_t));
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

void access_register_add_entry(access_register_t *reg, const char *username,
                               const char *destination, time_t timestamp) {
  if (!reg)
    return;
  access_entry_t *entry = &reg->entries[reg->tail];
  strncpy(entry->username, username, USERNAME_MAX_LEN - 1);
  entry->username[USERNAME_MAX_LEN - 1] = '\0';
  strncpy(entry->destination, destination, DEST_MAX_LEN - 1);
  entry->destination[DEST_MAX_LEN - 1] = '\0';
  entry->timestamp = timestamp;
  reg->tail = (reg->tail + 1) % REGISTER_MAX_ENTRIES;
  if (reg->count < REGISTER_MAX_ENTRIES) {
    reg->count++;
  } else {
    reg->head =
        (reg->head + 1) % REGISTER_MAX_ENTRIES; // sobrescribe el más antiguo
  }
  LOG(DEBUG, "Access entry added: user=%s, destination=%s", username,
      destination);
}

size_t access_register_print(const access_register_t *reg, char *response,
                             size_t response_size) {
  if (!reg)
    return 0;
  size_t idx = reg->head;
  size_t written = 0;
  int n = snprintf(response + written, response_size - written,
                   "%-20s %-25s "
                   "%-20s\n----------------------------------------------------"
                   "--------------\n",
                   "Usuario", "Destino", "Fecha");
  if (n < 0 || (size_t)n >= response_size - written)
    return written;
  written += n;
  for (size_t i = 0; i < reg->count; i++) {
    const access_entry_t *entry = &reg->entries[idx];
    char timebuf[32];
    struct tm *tm_info = localtime(&entry->timestamp);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
    n = snprintf(response + written, response_size - written,
                 "%-20s %-25s %-20s\n", entry->username, entry->destination,
                 timebuf);
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
