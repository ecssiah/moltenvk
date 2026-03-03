#ifndef APP_H
#define APP_H 1

#include "platform/platform.h"
#include "renderer/renderer.h"

typedef struct
{
    boolean is_running;
    Platform* platform;
    Renderer* renderer;
} 
App;

void app_init(App* app);
void app_start(App* app);
void app_update(App* app);
void app_destroy(App* app);

#endif