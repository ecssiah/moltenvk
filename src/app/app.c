#include "app/app.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core/log/log.h"
#include "render/render.h"
#include "platform/platform.h"

App* app_create()
{
    App* app = malloc(sizeof (*app));

    app->platform = platform_create();
    app->render = render_create();

    return app;
}

void app_destroy(App* app) 
{
    platform_destroy(app->platform);
    render_destroy(app->render);

    LOG_INFO("App Destroyed");
}

void app_init(App* app)
{
    app->is_running = true;

    platform_init(app->platform);
    render_init(app->render, app->platform);

    LOG_INFO("App Initialized");
}

void app_run(App* app)
{
    while (platform_is_active(app->platform))
    {
        platform_update(app->platform);
        render_update(app->render);
    }
}