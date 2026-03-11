#include "core/log/log.h"
#include "world_internal.h"

#include "platform/platform_internal.h"
#include <stdio.h>

World* world_create(void)
{
    World* world = malloc(sizeof(*world));

    return world;
}

void world_destroy(World* world)
{
    free(world);
}

void world_init(World* world)
{
    glm_vec3_zero(world->camera.position);
    glm_mat4_identity(world->camera.orientation);
}

void world_update(World* world, Platform* platform)
{
    vec3 input_value;
    glm_vec3_zero(input_value);

    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_W))
    {
        input_value[0] += 1.0f;
    }

    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_S))
    {
        input_value[0] -= 1.0f;
    }
    
    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_D))
    {
        input_value[1] += 1.0f;
    }

    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_A))
    {
        input_value[1] -= 1.0f;
    }
    
    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_E))
    {
        input_value[2] += 1.0f;
    }

    if (platform_is_key_down(&platform->platform_input, GLFW_KEY_Q))
    {
        input_value[2] -= 1.0f;
    }

    const float camera_speed = 1.0f;

    glm_vec3_scale(input_value, camera_speed, input_value);
    glm_vec3_add(input_value, world->camera.position, world->camera.position);

    glm_vec3_print(world->camera.position, stdout);
}