#include "core/core.h"
#include "core/log/log.h"

#include <stdio.h>
#include <stdlib.h>

size_t read_file_binary(const char* filename, char** out_buffer)
{
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        LOG_FATAL("Could not open: %s", filename);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size);

    if (!buffer)
    {
        fclose(file);

        LOG_FATAL("Failed to allocate shader buffer");
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    *out_buffer = buffer;

    return file_size;
}
