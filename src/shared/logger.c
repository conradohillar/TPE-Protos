
// logger.c
#include "logger.h"
#include <time.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_GRAY "\x1b[90m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_RESET "\x1b[0m"

static log_level_t current_level = LOG_DEBUG;

void log_set_level(log_level_t level) { current_level = level; }

static const char *level_to_string(log_level_t level) {
  switch (level) {
  case LOG_DEBUG:
    return "DEBUG";
  case LOG_INFO:
    return "INFO";
  case LOG_WARNING:
    return "WARN";
  case LOG_ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

static const char *level_to_color(log_level_t level) {
  switch (level) {
  case LOG_DEBUG:
    return ANSI_COLOR_BLUE;
  case LOG_INFO:
    return ANSI_COLOR_GREEN;
  case LOG_WARNING:
    return ANSI_COLOR_YELLOW;
  case LOG_ERROR:
    return ANSI_COLOR_RED;
  default:
    return ANSI_COLOR_RESET;
  }
}

static void log_print(log_level_t level, const char *file, const char *fmt,
                      va_list args) {
  if (level < current_level)
    return;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char timebuf[20];
  strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

  fprintf(stderr, "%s[%s] %s[%s]%s (%s): ", ANSI_COLOR_GRAY, timebuf,
          level_to_color(level), level_to_string(level), ANSI_COLOR_WHITE,
          file);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "%s\n", ANSI_COLOR_RESET);
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
