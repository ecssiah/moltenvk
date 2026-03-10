#include "core/math/math.h"

void perspective_lh(float fovy_radians, float aspect, float near_plane, float far_plane, mat4 out_projection_matrix)
{
    float f = 1.0f / tanf(fovy_radians * 0.5f);

    glm_mat4_zero(out_projection_matrix);

    out_projection_matrix[0][0] = f / aspect;
    out_projection_matrix[1][1] = f;

    out_projection_matrix[2][2] = far_plane / (far_plane - near_plane);
    out_projection_matrix[2][3] = 1.0f;

    out_projection_matrix[3][2] = -(near_plane * far_plane) / (far_plane - near_plane);
}

void perspective_rh(float fovy_radians, float aspect, float near_plane, float far_plane, mat4 out_projection_matrix)
{

}

