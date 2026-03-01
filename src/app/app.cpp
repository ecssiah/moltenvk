#include "app.h"

void app_init(App* app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    app->window = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        "Vulkan Test", 
        nullptr, 
        nullptr
    );

    app->render = {};
    app->render.window = app->window;

    render_init(&app->render);
}

void app_start(App* app)
{
    while (!glfwWindowShouldClose(app->window))
    {
        glfwPollEvents();

        render_frame(&app->render);
    }
}

void app_shutdown(App* app) 
{
    render_destroy(&app->render);

    glfwDestroyWindow(app->window);
    glfwTerminate();
}