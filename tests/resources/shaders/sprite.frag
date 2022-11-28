#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D textureSampler;

layout(location = 0) in vec2 in_texCoord;

// outColor linked to framebuffer 0
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = texture(textureSampler, in_texCoord);
    //out_color = vec4(1, 0, 0, 1);

    // no write to depth buffer
    if(out_color.a == 0)
        discard;
}
