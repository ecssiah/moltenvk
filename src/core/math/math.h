#ifndef MATH_H
#define MATH_H 1

#include <cglm/cglm.h>

void look_at_lh(vec3 eye, vec3 center, vec3 up, mat4 out_view_matrix);
void look_at_rh(vec3 eye, vec3 center, vec3 up, mat4 out_view_matrix);

void perspective_lh(float fovy_radians, float aspect, float near_plane, float far_plane, mat4 out_projection_matrix);
void perspective_rh(float fovy_radians, float aspect, float near_plane, float far_plane, mat4 out_projection_matrix);

#endif