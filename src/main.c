#include "nuklear/nuklear.h"

#include "app/app.h"
#include "core/log/log.h"

int main(int argc, char** argv)
{
    log_init();

    App* app = app_create();

    app_init(app);
    app_run(app);
    app_destroy(app);

    log_shutdown();
}