#include "app/world/world.h"

#include <stdio.h>

#include "app/camera.h"
#include "core/log/log.h"
#include "platform/platform.h"

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

}

void world_update(World* world, Platform* platform, f64 delta_time)
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

    vec3 forward;
    camera_get_forward(&world->camera, forward);

    vec3 right;
    camera_get_right(&world->camera, right);

    vec3 up;
    camera_get_up(&world->camera, up);
    
    const f32 camera_speed = 1.0f;
    vec3 camera_position_delta = {0.0f, 0.0f, 0.0f};

    vec3 velocity_forward;
    glm_vec3_scale(forward, input_value[0] * camera_speed * delta_time, velocity_forward);
    glm_vec3_add(camera_position_delta, velocity_forward, camera_position_delta);

    vec3 velocity_right;
    glm_vec3_scale(right, input_value[1] * camera_speed * delta_time, velocity_right);
    glm_vec3_add(camera_position_delta, velocity_right, camera_position_delta);

    vec3 velocity_up;
    glm_vec3_scale(up, input_value[2] * camera_speed * delta_time, velocity_up);
    glm_vec3_add(camera_position_delta, velocity_up, camera_position_delta);

    glm_vec3_add(world->camera.position, camera_position_delta, world->camera.position);

    const f32 sensitivity = 12.0f;

    const f64 mouse_delta_x = platform_mouse_delta_x(&platform->platform_input);
    const f64 mouse_delta_y = platform_mouse_delta_y(&platform->platform_input);

    if (mouse_delta_x < 50.0f)
    {
        camera_set_rotation_z(
            &world->camera, 
            world->camera.rotation_angles[2] - mouse_delta_x * sensitivity * delta_time
        );
    }

    if (mouse_delta_y < 50.0f)
    {
        camera_set_rotation_y(
            &world->camera, 
            world->camera.rotation_angles[1] - mouse_delta_y * sensitivity * delta_time
        );
    }
}