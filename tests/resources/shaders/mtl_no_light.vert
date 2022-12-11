#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PCST
{
    mat4 mvp;
} pcst;

layout(location = 0) in vec3 position;

void main()
{
    gl_Position = pcst.mvp * vec4(position, 1);
}
