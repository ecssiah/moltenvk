#include "app/app.h"
#include "core/log.h"

int main(int argc, char** argv)
{
    log_init();

    App app = 
    {
        .is_running = false,
    };

    app_init(&app);

    LOG_INFO("Starting App");

    app_start(&app);
    app_destroy(&app);

    log_shutdown();
}