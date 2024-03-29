#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D textureSampler;

layout(location = 0) in vec4 in_lightColor;
layout(location = 1) in vec3 in_specular;
layout(location = 2) in vec2 in_texCoord;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = texture(textureSampler, in_texCoord) * in_lightColor;
    out_color += vec4(in_specular, 0);

    if(out_color.a == 0)
        discard;
}
