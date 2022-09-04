#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform pushConstants
{
    float alpha;
} cst;

// inColor comming from vertex shader
layout(location = 0) in vec3 fragColor;

// outColor linked to framebuffer 0
layout(location = 0) out vec4 outColor;

void main()
{
    // alpha set to 1 for no transparency
    outColor = vec4(fragColor, cst.alpha);
    //outColor = vec4(fragColor, 0.3);
}
