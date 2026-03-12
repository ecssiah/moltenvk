#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    mat4 orientation;
}
Camera;

void camera_get_forward(Camera* camera, vec3 out_forward);

#endif