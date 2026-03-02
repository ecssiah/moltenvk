#include "app.h"

#include <stdio.h>
#include <string.h>

void app_init(App* app)
{
    memset(app, 0, sizeof (*app));

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    app->window = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        "Vulkan Test", 
        NULL, 
        NULL
    );

    render_init(&app->render, app->window);
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