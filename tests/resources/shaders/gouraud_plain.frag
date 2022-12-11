#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 in_lightColor;
layout(location = 1) in vec3 in_specular;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = in_lightColor + vec4(in_specular, 0);

    if(out_color.a == 0)
        discard;
}
