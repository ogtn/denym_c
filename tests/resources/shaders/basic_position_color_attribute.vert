#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main()
{
    // z = 0, w = 1 so transformation to clip coordinate will be a non op
    gl_Position = vec4(position, 0, 1);

    fragColor = color;
}
