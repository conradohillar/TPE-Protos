#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum { DEBUG, INFO, WARNING, ERROR } LogLevel;

void init_logging(const char* filename);
void close_logging();
void set_log_level(LogLevel level);
void log_message(LogLevel level, const char* text);
void log_messagef(LogLevel level, const char* file, int line, const char* fmt, ...);

#define LOG(level, fmt, ...) log_messagef(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_MSG(level, msg)  log_messagef(level, __FILE__, __LINE__, "%s", msg)

#endif // LOGGER_H
