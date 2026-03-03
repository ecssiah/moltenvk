#include "app/app.h"

#include "core/log.h"

int main()
{
    log_init("logs/engine.log");

    App app;
    app.is_running = false;
    app.platform = NULL;
    app.renderer = NULL;

    app_init(&app);

    LOG_INFO("Starting App");

    app_start(&app);
    app_destroy(&app);

    log_shutdown();
}