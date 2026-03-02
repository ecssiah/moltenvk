#ifndef APP_H
#define APP_H 1

#include "platform/render.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

typedef struct
{
    GLFWwindow* window;
    Render render;
} 
App;

void a_init(App* app);
void a_start(App* app);
void a_quit(App* app);

#endif