#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) readonly buffer SSBO
{
    mat4 mvp[];
} ssbo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 out_texCoord;

void main()
{
    gl_Position = ssbo.mvp[gl_InstanceIndex] * vec4(position, 1);

    out_texCoord = texCoord;
}
