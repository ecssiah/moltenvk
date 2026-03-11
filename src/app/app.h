#ifndef APP_H
#define APP_H 1

#include <stdbool.h>

typedef struct Platform Platform;
typedef struct Render Render;

typedef struct
{
    bool is_running;

    Platform* platform;
    Render* render;
} 
App;

App* app_create();
void app_destroy(App* app);
void app_init(App* app);

void app_run(App* app);
void app_update(App* app);

#endif