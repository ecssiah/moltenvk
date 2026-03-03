#include "app.h"
#include "platform/platform.h"

#include <stdio.h>
#include <string.h>

#include "core/log.h"

void app_init(App* app)
{
    app->platform = platform_create();
    app->renderer = renderer_create(app->platform);
    
    app->is_running = true;

    LOG_INFO("App Initialized");
}

void app_start(App* app)
{
    while (!platform_should_close(app->platform))
    {
        app_update(app);
    }
}

void app_update(App* app)
{
    platform_poll_events(app->platform);
    platform_handle_inputs(app->platform);

    renderer_draw(app->renderer);
}

void app_destroy(App* app) 
{
    renderer_destroy(app->renderer);
    platform_destroy(app->platform);

    LOG_INFO("App Destroyed");
}