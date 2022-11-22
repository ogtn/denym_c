#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_texCoord;
layout(location = 2) out vec4 out_position;

void main()
{
    // put everything in eye coordinates
    mat4 modelViewMatrix = ubo.view * ubo.model;
    vec4 pos = modelViewMatrix * vec4(in_position, 1);
    mat3 normalMatrix = mat3(modelViewMatrix);

    out_normal = normalMatrix * in_normal;
    out_texCoord = in_texCoord;
    out_position = pos;
    gl_Position = ubo.proj * pos;
}
