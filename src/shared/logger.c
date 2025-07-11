#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_PURPLE "\x1b[35m"
#define COLOR_GRAY "\x1b[90m"
#define COLOR_WHITE "\x1b[37m"
#define COLOR_RESET "\x1b[0m"

static log_level_t current_log_level = DEBUG;
static FILE* log_file = NULL;

void init_logging(const char* filename, log_level_t level) {
    current_log_level = level;
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", filename);
            exit(EXIT_FAILURE);
        }
    } else {
        log_file = stdout;
    }
    if (filename && log_file) {
        chmod(filename, S_IRUSR | S_IWUSR); // Owner can read and write
    }
}

void close_logging(void) {
    if (log_file && log_file != stdout) {
        fclose(log_file);
        log_file = NULL;
    }
}

void set_log_level(log_level_t level) { current_log_level = level; }

void log_message(log_level_t level, const char* text) {
    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }
    if (level < current_log_level)
        return;

    const char* level_strings[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    const char* level_colors[] = {COLOR_PURPLE, COLOR_BLUE, COLOR_YELLOW,
                                  COLOR_RED};

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    if (log_file == stdout) {
        fprintf(log_file, "%s[%s]%s[%s%s%s] %s%s%s\n", COLOR_GRAY, timebuf,
                COLOR_RESET, level_colors[level], level_strings[level], COLOR_RESET,
                COLOR_WHITE, text, COLOR_RESET);
    } else {
        fprintf(log_file, "[%s][%s] %s\n", timebuf, level_strings[level], text);
    }
    fflush(log_file);
}

void log_messagef(log_level_t level, const char* file, int line, const char* fmt,
                  ...) {
    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }
    if (level < current_log_level)
        return;

    const char* level_strings[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    const char* level_colors[] = {COLOR_PURPLE, COLOR_BLUE, COLOR_YELLOW,
                                  COLOR_RED};

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    if (log_file == stdout) {
        fprintf(log_file, "%s[%s]%s[%s%s%s][%s:%d] %s", COLOR_GRAY, timebuf,
                COLOR_RESET, level_colors[level], level_strings[level], COLOR_RESET,
                file, line, COLOR_WHITE);
    } else {
        fprintf(log_file, "[%s][%s][%s:%d] ", timebuf, level_strings[level], file,
                line);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    if (log_file == stdout) {
        fprintf(log_file, "%s\n", COLOR_RESET);
    } else {
        fprintf(log_file, "\n");
    }
    fflush(log_file);
}
