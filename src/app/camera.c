#include "app/camera.h"

#include "core/types.h"

void camera_init(Camera* camera)
{
    glm_vec3_zero(camera->position);

    camera->position[0] = -16.0f;
    camera->position[1] = 0.0f;
    camera->position[2] = 0.0f;

    glm_vec3_zero(camera->rotation_angles);
}

void camera_set_rotation_x(Camera* camera, f32 angle_degrees)
{
    camera->rotation_angles[0] = angle_degrees;
}

void camera_set_rotation_y(Camera* camera, f32 angle_degrees)
{
    if (angle_degrees > 89.9f)
    {
        camera->rotation_angles[1] = 89.9f;
    }
    else if (angle_degrees < -89.9f)
    {
        camera->rotation_angles[1] = -89.9f;
    }
    else
    {
        camera->rotation_angles[1] = angle_degrees;
    }
}

void camera_set_rotation_z(Camera* camera, f32 angle_degrees)
{
    camera->rotation_angles[2] = angle_degrees;
}

void camera_get_forward(Camera* camera, vec3 out_forward)
{
    const vec3 rotation_angles_radians = 
    { 
        glm_rad(camera->rotation_angles[0]),
        glm_rad(camera->rotation_angles[1]),
        glm_rad(camera->rotation_angles[2])
    };

    out_forward[0] = cos(rotation_angles_radians[1]) * cos(rotation_angles_radians[2]);
    out_forward[1] = cos(rotation_angles_radians[1]) * sin(rotation_angles_radians[2]);
    out_forward[2] = sin(rotation_angles_radians[1]);
}

void camera_get_right(Camera* camera, vec3 out_right)
{
    vec3 forward;
    camera_get_forward(camera, forward);

    glm_vec3_cross(forward, GLM_ZUP, out_right);
    glm_vec3_normalize(out_right);
}

void camera_get_up(Camera* camera, vec3 out_up)
{
    vec3 right, forward;

    camera_get_forward(camera, forward);
    camera_get_right(camera, right);

    glm_vec3_cross(right, forward, out_up);
}

void camera_get_orientation(Camera* camera, mat4 out_orientation)
{
    vec3 forward, right, up;
    camera_get_forward(camera, forward);
    camera_get_right(camera, right);
    camera_get_up(camera, up);

    glm_mat4_zero(out_orientation);

    out_orientation[0][0] = forward[0];
    out_orientation[1][0] = forward[1];
    out_orientation[2][0] = forward[2];

    out_orientation[0][1] = right[0];
    out_orientation[1][1] = right[1];
    out_orientation[2][1] = right[2];

    out_orientation[0][2] = up[0];
    out_orientation[1][2] = up[1];
    out_orientation[2][2] = up[2];
}