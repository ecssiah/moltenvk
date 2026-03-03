#include "app/app.h"

#include <stdio.h>
#include <string.h>

#include "core/log.h"
#include "platform/platform.h"
#include "platform/platform_internal.h"
#include "renderer/renderer.h"

void app_init(App* app)
{
    app->platform = platform_create();
    app->renderer = renderer_create(app->platform);
    
    app->is_running = true;

    LOG_INFO("App Initialized");
}

void app_start(App* app)
{
    while (!app->platform->platform_window.close_requested)
    {
        platform_update(app->platform);
        renderer_update(app->renderer);
    }
}

void app_destroy(App* app) 
{
    platform_destroy(app->platform);
    renderer_destroy(app->renderer);

    LOG_INFO("App Destroyed");
}