
// logger.c
#include "logger.h"
#include <time.h>

static log_level_t current_level = LOG_DEBUG;

void log_set_level(log_level_t level) {
    current_level = level;
}

static const char* level_to_string(log_level_t level) {
    switch(level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static void log_print(log_level_t level, const char *file, const char *fmt, va_list args) {
    if (level < current_level) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(stderr, "[%s] [%s] (%s): ", timebuf, level_to_string(level), file);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

void _log_debug(const char *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_print(LOG_DEBUG, file, fmt, args);
    va_end(args);
}

void _log_info(const char *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_print(LOG_INFO, file, fmt, args);
    va_end(args);
}

void _log_warning(const char *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_print(LOG_WARNING, file, fmt, args);
    va_end(args);
}

void _log_error(const char *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_print(LOG_ERROR, file, fmt, args);
    va_end(args);
}
