#ifndef APP_H
#define APP_H 1

#include <stdbool.h>

#include "core/types.h"

typedef struct Platform Platform;
typedef struct Render Render;
typedef struct World World;

typedef struct
{
    bool is_running;

    f64 last_time;
    f64 delta_time;

    Platform* platform;
    Render* render;

    World* world;
}
App;

App* app_create(void);
void app_destroy(App* app);

void app_init(App* app);
void app_run(App* app);
void app_update(App* app);

#endif