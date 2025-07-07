#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdio.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR } log_level_t;

void log_set_level(log_level_t level);

void _log_debug(const char *file, const char *fmt, ...);
void _log_info(const char *file, const char *fmt, ...);
void _log_warning(const char *file, const char *fmt, ...);
void _log_error(const char *file, const char *fmt, ...);

#define log_debug(...) _log_debug(__FILE__, __VA_ARGS__)
#define log_info(...) _log_info(__FILE__, __VA_ARGS__)
#define log_warning(...) _log_warning(__FILE__, __VA_ARGS__)
#define log_error(...) _log_error(__FILE__, __VA_ARGS__)

#endif // LOGGER_H