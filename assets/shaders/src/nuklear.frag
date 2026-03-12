#version 450

layout(binding = 0) 
uniform sampler2D texture;

layout(location = 0) 
in vec2 frag_uv;

layout(location = 1) 
in vec4 frag_color;

layout(location = 0) 
out vec4 out_color;

void main()
{
    vec4 texture_color = texture(texture, frag_uv);

    out_color = frag_color * texture_color;
}