#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

#include "core/types.h"

typedef struct {
    vec3 position;
    vec3 rotation_angles;

    mat4 orientation;
}
Camera;

void camera_init(Camera* camera);

void camera_set_rotation_x(Camera* camera, f32 angle_degrees);
void camera_set_rotation_y(Camera* camera, f32 angle_degrees);
void camera_set_rotation_z(Camera* camera, f32 angle_degrees);

void camera_get_forward(Camera* camera, vec3 out_forward);
void camera_get_right(Camera* camera, vec3 out_right);
void camera_get_up(Camera* camera, vec3 out_up);

void camera_get_orientation(Camera* camera, mat4 out_orientation);

#endif