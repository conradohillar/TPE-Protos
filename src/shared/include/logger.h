#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>



typedef enum log_level {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
} log_level_t;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

static inline const char *log_level_str(int level) {
    switch (level) {
        case LOG_ERROR:   return "ERROR";
        case LOG_WARNING: return "WARNING";
        case LOG_INFO:    return "INFO";
        case LOG_DEBUG:   return "DEBUG";
        default:          return "UNKNOWN";
    }
}

static inline void log_write(int level, const char *file, int line, const char *fmt, ...) {
    if (level > LOG_LEVEL) return;

    va_list args;
    va_start(args, fmt);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(stderr, "[%s] %s (%s:%d): ", timebuf, log_level_str(level), file, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}

#define log_error(fmt, ...)   do { if (LOG_LEVEL >= LOG_ERROR)   log_write(LOG_ERROR,   __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define log_warning(fmt, ...) do { if (LOG_LEVEL >= LOG_WARNING) log_write(LOG_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define log_info(fmt, ...)    do { if (LOG_LEVEL >= LOG_INFO)    log_write(LOG_INFO,    __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define log_debug(fmt, ...)   do { if (LOG_LEVEL >= LOG_DEBUG)   log_write(LOG_DEBUG,   __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#endif // LOGGER_H