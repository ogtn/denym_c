#version 450
#extension GL_ARB_separate_shader_objects : enable

struct dlight_t
{
    vec3 direction;
    float intensity;
    vec3 color;
    float ambiant;
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

layout(binding = 1) uniform LIGHTS
{
    light_t light_0;
} lights;

layout(binding = 2) uniform sampler2D textureSampler;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec4 in_position;

layout(location = 0) out vec4 out_color;

void main()
{
    // single directionnal light
    light_t light = lights.light_0;

    // setup hardcoded material
    material_t material;
    material.color = vec3(1, 1, 1);
    material.shininess = 100;

    // ambiant light
    vec3 color = /*material.color * */light.color * light.ambiant;

    vec3 normal = normalize(in_normal);
    vec3 lightDirection = normalize((ubo.view * vec4(light.direction, 0)).xyz);
    float lightAngle = max(dot(normal, lightDirection), 0);
    vec4 specular = vec4(0);

    if(lightAngle > 0)
    {
        // diffuse light
        color += material.color * light.color * light.intensity * lightAngle;

        vec3 eye = -vec3(in_position);
        vec3 halfVector = normalize(lightDirection + eye);
        float halfAngle = max(dot(normal, halfVector), 0);

        // specular light
        specular = vec4(/*material.color * */ light.color * light.intensity * pow(halfAngle, material.shininess), 1);
    }

    out_color = texture(textureSampler, in_texCoord) * vec4(color, 1) + specular;

    if(out_color.a == 0)
        discard;
}
