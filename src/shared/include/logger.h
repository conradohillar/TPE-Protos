#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum { 
    DEBUG,
    INFO,
    WARNING,
    ERROR 
} log_level_t;

void init_logging(const char* filename, log_level_t level);
void close_logging();
void set_log_level(log_level_t level);
char *get_log_level_string();
void log_message(log_level_t level, const char* text);
void log_messagef(log_level_t level, const char* file, int line, const char* fmt, ...);

#define LOG(level, fmt, ...) log_messagef(level, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_MSG(level, msg) log_messagef(level, __FILE__, __LINE__, "%s", msg)

#endif // LOGGER_H
