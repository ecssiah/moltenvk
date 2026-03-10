#version 450

layout(push_constant) 
uniform Push
{
    mat4 projection_view_matrix;
}
push;

layout(location = 0) 
in vec3 in_position;

layout(location = 1) 
in vec2 in_uv;

layout(location = 0) 
out vec2 frag_uv;

void main()
{
    gl_Position = push.projection_view_matrix * vec4(in_position, 1.0);

    frag_uv = in_uv;
}