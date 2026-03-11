#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    mat4 orientation;
}
Camera;

#endif