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
    dlight_t light_0;
} lights;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_lightColor;
layout(location = 1) out vec3 out_specular;
layout(location = 2) out vec2 out_texCoord;


void computeDirectionalLightColor(dlight_t light, material_t material, vec4 pos, vec3 normal, out vec3 color, out vec3 specular)
{
    color = specular = vec3(0);

    if(light.intensity == 0)
        return;

    // ambiant light
    color += material.color * light.color * light.ambiant;

    vec3 lightDirection = normalize(vec3(ubo.view * vec4(light.direction, 0)));
    float lightAngle = max(dot(normal, lightDirection), 0);

    if(lightAngle > 0)
    {
        // diffuse light
        color += material.color * light.color * light.intensity * lightAngle;

        vec3 eye = normalize(-vec3(pos));
        vec3 halfVector = normalize(lightDirection + eye);
        float halfAngle = max(dot(normal, halfVector), 0);

        // specular light
        specular = /*material.color * */ light.color * light.intensity * pow(halfAngle, material.shininess);
    }
}


void main()
{
    // setup hardcoded material
    material_t material;
    material.color = vec3(1, 1, 1);
    material.shininess = 100;

    // put everything in eye coordinates
    mat4 modelViewMatrix = ubo.view * ubo.model;
    vec4 pos = modelViewMatrix * vec4(in_position, 1);
    mat3 normalMatrix = mat3(modelViewMatrix);
    vec3 normal = normalize(normalMatrix * in_normal);

    vec3 color, specular;

    // single directionnal light
    computeDirectionalLightColor(dlights.light_0, material, pos, normal, color, specular);
    out_lightColor = vec4(color, 1);
    out_specular = specular;

    out_texCoord = in_texCoord;
    gl_Position = ubo.proj * pos;
}
