#include <stdint.h>

#include "stb/stb_image.h"

#include "core/types.h"
#include "render/render.h"
#include "render/render_internal.h"

Image render_image_load(const char* path)
{
    Image image;

    int width;
    int height;
    int channels;

    u8* pixels = stbi_load(
        path,
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!pixels)
    {
        image.width = 0;
        image.height = 0;
        image.channels = 0;
        image.pixels = NULL;
        return image;
    }

    image.width = (u32)width;
    image.height = (u32)height;
    image.channels = 4;
    image.pixels = pixels;

    return image;
}

void render_image_destroy(Image* image)
{
    if (image->pixels)
    {
        stbi_image_free(image->pixels);
        image->pixels = NULL;
    }
}
