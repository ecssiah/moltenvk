#include "core/core.h"

#include <stdio.h>
#include <stdlib.h>

size_t c_read_file_binary(const char* filename, char** out_buffer)
{
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        fprintf(stderr, "Could not open: %s\n", filename);

        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size);

    if (!buffer)
    {
        fclose(file);

        fprintf(stderr, "Failed to allocate shader buffer\n");

        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    *out_buffer = buffer;

    return file_size;
}
