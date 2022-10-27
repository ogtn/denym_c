#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PCST
{
    mat4 mvp;
} pcst;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 out_texCoord;

void main()
{
    gl_Position = pcst.mvp * vec4(position, 1);

    out_texCoord = texCoord;
}
