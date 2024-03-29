#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform MVP
{
    mat4 mvp;
} ubo;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

void main()
{
    gl_Position = ubo.mvp * vec4(in_position, 0, 1);

    out_color = in_color;
}
