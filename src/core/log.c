#include "core/log.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static FILE* g_log_file = NULL;
static char g_log_base_path[256] = {0};
static char g_current_day[11] = {0};

static const char* log_level_strings[] =
{
    "TRACE",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

void log_init()
{
    const char* directory_name = "logs/";

    strncpy(g_log_base_path, directory_name, sizeof(g_log_base_path) - 1);

    if (!g_log_file)
    {
        g_log_file = stderr;
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

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    char file_timestamp[11];
    strftime(file_timestamp, sizeof(file_timestamp), "%Y_%m_%d", &tm_info);

    if (g_log_base_path[0] != '\0' && strcmp(file_timestamp, g_current_day) != 0)
    {
        if (g_log_file && g_log_file != stderr)
        {
            fclose(g_log_file);
        }

        strncpy(g_current_day, file_timestamp, sizeof(g_current_day) - 1);

        char path[512];
        snprintf(path, sizeof(path), "%sengine_%s.log", g_log_base_path, file_timestamp);

        g_log_file = fopen(path, "a");
        if (!g_log_file)
        {
            g_log_file = stderr;
        }
    }

    const char* filename = file;
    const char* last_slash = strrchr(file, '/');

    if (last_slash) 
    {
        filename = last_slash + 1;
    }

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