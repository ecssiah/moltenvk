#include "app/camera.h"

void camera_get_forward(Camera* camera, vec3 out_forward)
{
    out_forward[0] = -camera->orientation[2][0];
    out_forward[1] = -camera->orientation[2][1];
    out_forward[2] = -camera->orientation[2][2];
}