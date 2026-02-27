#include "app.h"

void App::init()
{
    renderer.init();
}

void App::start()
{
    while (!glfwWindowShouldClose(renderer.window))
    {
        glfwPollEvents();

        renderer.draw_frame();
    }

    renderer.wait_idle();
}