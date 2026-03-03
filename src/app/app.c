#include "app.h"

#include <stdio.h>
#include <string.h>

void app_init(App* app)
{
    app->platform_window = platform_window_create();
    app->renderer = renderer_create(app->platform_window);
    
    app->is_running = true;
}

void app_start(App* app)
{
    while (!platform_window_should_close(app->platform_window))
    {
        platform_window_poll_events(app->platform_window);

        // platform_window_is_key_pressed()

        renderer_draw(app->renderer);
    }
}

void app_destroy(App* app) 
{
    renderer_destroy(app->renderer);
    platform_window_destroy(app->platform_window);
}