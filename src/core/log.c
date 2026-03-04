#include "core/log.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static FILE* g_log_file = NULL;

static const char* log_level_strings[] =
{
    "TRACE",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

void log_init(const char* filepath)
{
    if (!g_log_file)
    {
        g_log_file = stderr;
    }
    
    if (filepath)
    {
        g_log_file = fopen(filepath, "a");
    }

    LOG_INFO("\n\nLOGGING INIT\n");
}

void log_message(
    LogLevel level,
    const char* file,
    int line,
    const char* fmt,
    ...
) {
    if (!g_log_file)
    {
        g_log_file = stderr;
    }

    const char* filename = file;
    const char* last_slash = strrchr(file, '/');

    if (last_slash) 
    {
        filename = last_slash + 1;
    }

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_info);

    fprintf(
        stderr, 
        "[%s] [%s] (%s:%d) ",
        timestamp,
        log_level_strings[level],
        filename,
        line
    );

    if (g_log_file && g_log_file != stderr) 
    {
        fprintf(
            g_log_file, 
            "[%s] [%s] (%s:%d) ",
            timestamp,
            log_level_strings[level],
            filename,
            line
        );
    }

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    vfprintf(stderr, fmt, args);

    if (g_log_file && g_log_file != stderr)
    {
        vfprintf(g_log_file, fmt, args_copy);
    }

    va_end(args_copy);
    va_end(args);

    fprintf(stderr, "\n");

    if (g_log_file && g_log_file != stderr) 
    {
        fprintf(g_log_file, "\n");
        fflush(g_log_file);
    }

    if (level == LOG_LEVEL_FATAL)
    {
        fflush(stderr);

        if (g_log_file && g_log_file != stderr) 
        {
            fflush(g_log_file);
        }

        exit(EXIT_FAILURE);
    }
}

void log_shutdown(void)
{
    LOG_INFO("\n\nLOGGING SHUTDOWN\n");

    if (g_log_file && g_log_file != stderr)
    {
        fclose(g_log_file);
    }
}