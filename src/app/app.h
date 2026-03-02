#ifndef APP_H
#define APP_H

#include "../platform/render.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

typedef struct
{
    GLFWwindow* window;
    Render render;
} 
App;

void app_init(App* app);
void app_start(App* app);
void app_shutdown(App* app);

#endif