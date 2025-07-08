#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

static LogLevel current_log_level = INFO;
static FILE* log_file = NULL;

void init_logging(const char* filename) {
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

void close_logging() {
    if (log_file && log_file != stdout) {
        fclose(log_file);
        log_file = NULL;
    }
}

void set_log_level(LogLevel level) {
    current_log_level = level;
}

void log_message(LogLevel level, const char* text) {
    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }
    if (level < current_log_level) return;

    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR" };
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log_file, "[%s][%s] %s\n", timebuf, level_strings[level], text);
    fflush(log_file);
}

void log_messagef(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }
    if (level < current_log_level) return;

    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR" };
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log_file, "[%s][%s][%s:%d] ", timebuf, level_strings[level], file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
}
