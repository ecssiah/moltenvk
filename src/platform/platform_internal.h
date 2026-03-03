#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H 1

#include <GLFW/glfw3.h>

#include "core/types.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

struct PlatformWindow
{
    struct GLFWwindow* handle;
    u32 width;
    u32 height;
    boolean should_close;
};

#endif