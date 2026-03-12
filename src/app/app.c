#include "app/app.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "core/log/log.h"
#include "render/render.h"
#include "platform/platform.h"
#include "app/world/world.h"

App* app_create(void)
{
    App* app = malloc(sizeof (*app));

    app->platform = platform_create();
    app->render = render_create(app->platform);

    app->world = world_create();

    return app;
}

void app_destroy(App* app) 
{
    platform_destroy(app->platform);
    render_destroy(app->render);

    world_destroy(app->world);

    LOG_INFO("App Destroyed");
}

void app_init(App* app)
{
    app->is_running = true;

    app->last_time = glfwGetTime();
    app->delta_time = 0.0;

    platform_init(app->platform);
    render_init(app->render, app->platform);

    world_init(app->world);

    LOG_INFO("App Initialized");
}

void app_run(App* app)
{
    while (platform_is_active(app->platform))
    {
        const double current_time = glfwGetTime();
        
        f64 delta_time = current_time - app->last_time;
        
        if (delta_time > 0.1)
        {
            delta_time = 0.1;
        }

        app->delta_time = delta_time;
        app->last_time = current_time;

        platform_update(app->platform);
        world_update(app->world, app->platform, app->delta_time);
        render_update(app->render, app->world, app->delta_time);

        render_draw(app->render);
    }
}