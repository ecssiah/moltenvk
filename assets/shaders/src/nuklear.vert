#version 450

layout(location = 0) 
in vec2 in_position;

layout(location = 1) 
in vec2 in_uv;

layout(location = 2) 
in vec4 in_color;

layout(location = 0) 
out vec2 frag_uv;

layout(location = 1) 
out vec4 frag_color;

layout(push_constant) 
uniform Push 
{
    vec2 scale;
    vec2 translate;
} 
push;

void main()
{
    frag_uv = in_uv;
    frag_color = in_color;

    vec2 position = in_position * push.scale + push.translate;
    gl_Position = vec4(position, 0.0, 1.0);
}