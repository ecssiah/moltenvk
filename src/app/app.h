#pragma ounce

#include "../platform/render.h"

static constexpr int WINDOW_WIDTH = 1024;
static constexpr int WINDOW_HEIGHT = 768;

struct App
{
    GLFWwindow* window;
    Render render;
};

void app_init(App* app);
void app_start(App* app);
void app_shutdown(App* app);