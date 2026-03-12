#include "core/math/math.h"

void look_at_lh(vec3 eye, vec3 center, vec3 up, mat4 out_view_matrix)
{
    vec3 forward;
    glm_vec3_sub(center, eye, forward);
    glm_vec3_normalize(forward);

    vec3 right;
    glm_vec3_cross(forward, up, right);
    glm_vec3_normalize(right);

    vec3 view_up;
    glm_vec3_cross(right, forward, view_up);

    glm_mat4_identity(out_view_matrix);

    out_view_matrix[0][0] = right[0];
    out_view_matrix[1][0] = right[1];
    out_view_matrix[2][0] = right[2];

    out_view_matrix[0][1] = view_up[0];
    out_view_matrix[1][1] = view_up[1];
    out_view_matrix[2][1] = view_up[2];

    out_view_matrix[0][2] = forward[0];
    out_view_matrix[1][2] = forward[1];
    out_view_matrix[2][2] = forward[2];

    out_view_matrix[3][0] = -glm_vec3_dot(right, eye);
    out_view_matrix[3][1] = -glm_vec3_dot(view_up, eye);
    out_view_matrix[3][2] = -glm_vec3_dot(forward, eye);
}