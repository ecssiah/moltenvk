#include "app.h"

#include <stdio.h>
#include <string.h>

void a_init(App* app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    app->window = glfwCreateWindow(
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        "Vulkan Test", 
        NULL, 
        NULL
    );

    r_init(&app->render, app->window);
}

void a_start(App* app)
{
    while (!glfwWindowShouldClose(app->window))
    {
        glfwPollEvents();

        if (glfwGetKey(app->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(app->window, GLFW_TRUE);
        }

        r_render(&app->render);
    }
}

void a_quit(App* app) 
{
    r_quit(&app->render);

    glfwDestroyWindow(app->window);
    glfwTerminate();
}