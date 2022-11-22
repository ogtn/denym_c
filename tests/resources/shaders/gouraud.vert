#version 450
#extension GL_ARB_separate_shader_objects : enable

struct light_t
{
    vec3 direction;
    vec3 color;
    float intensity;
};

struct material_t
{
    vec3 color;
    float shininess;
};

layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_texCoord;
layout(location = 2) out vec3 out_specular;

void main()
{
    // setup hardcoded directional light
    light_t light;
    light.direction = vec3(1, 0, 1);
    light.color = vec3(1, 1, 1);
    light.intensity = 1;

    // setup hardcoded material
    material_t material;
    material.color = vec3(1, 1, 1);
    material.shininess = 100;

    // ambiant light
    vec3 color = material.color * light.color * 0.005;

    // put everything in eye coordinates
    mat4 modelViewMatrix = ubo.view * ubo.model;
    vec4 pos = modelViewMatrix * vec4(in_position, 1);
    mat3 normalMatrix = mat3(modelViewMatrix);
    vec3 normal = normalize(normalMatrix * in_normal);
    vec3 lightDirection = normalize(vec3(ubo.view * vec4(light.direction, 0)));
    float lightAngle = max(dot(normal, lightDirection), 0);

    if(lightAngle > 0)
    {
        // diffuse light
        color += material.color * light.color * light.intensity * lightAngle;

        vec3 eye = -vec3(pos);
        vec3 halfVector = normalize(lightDirection + eye);
        float halfAngle = max(dot(normal, halfVector), 0);

        // specular light
        out_specular = /*material.color * */ light.color * light.intensity * pow(halfAngle, material.shininess);
    }

    out_color = vec4(color, 1);
    out_texCoord = in_texCoord;
    gl_Position = ubo.proj * pos;
}
