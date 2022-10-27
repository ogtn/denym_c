#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec2 texCoord;

// outColor linked to framebuffer 0
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(textureSampler, texCoord);

    // no write to depth buffer
    if(outColor.a == 0)
        discard;
}
