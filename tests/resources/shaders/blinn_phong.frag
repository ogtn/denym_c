#version 450
#extension GL_ARB_separate_shader_objects : enable

struct dlight_t
{
    vec3 direction;
    float intensity;
    vec3 color;
    float ambiant;
};

struct plight_t
{
    vec3 position;
    float intensity;
    vec3 color;
    float ambiant;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
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

layout(binding = 1) uniform DLIGHTS
{
    dlight_t light_0;
} dlights;

layout(binding = 2) uniform PLIGHTS
{
    plight_t light[2];
} plights;

layout(binding = 3) uniform sampler2D textureSampler;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec4 in_position;

layout(location = 0) out vec4 out_color;


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


void computePointLightColor(plight_t light, material_t material, vec4 pos, vec3 normal, out vec3 color, out vec3 specular)
{
    color = specular = vec3(0);

    if(light.intensity == 0)
        return;

    // ambiant light
    color += material.color * light.color * light.ambiant;

    vec4 lightPosition = ubo.view * vec4(light.position, 1);
    vec3 lightDirection = (lightPosition - pos).xyz;
    float lightDst = length(lightDirection.xyz);
    lightDirection = normalize(lightDirection);
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

    float attenuation = 1 / (
    light.constantAttenuation +
    light.linearAttenuation * lightDst +
    light.quadraticAttenuation * lightDst * lightDst);

    color *= attenuation;
    specular *= attenuation;
}


void main()
{
    // setup hardcoded material
    material_t material;
    material.color = vec3(1, 1, 1);
    material.shininess = 100;

    vec3 color, specular;
    vec3 lightColor, specularColor;

    vec3 normal = normalize(in_normal);

    // single directionnal light
    computeDirectionalLightColor(dlights.light_0, material, in_position, normal, color, specular);
    lightColor = color;
    specularColor = specular;

    for(int i = 0; i < 2; i++)
    {
        computePointLightColor(plights.light[i], material, in_position, normal, color, specular);
        lightColor += color;
        specularColor += specular;
    }

    out_color = texture(textureSampler, in_texCoord) * vec4(lightColor, 1) + vec4(specularColor, 0);

    if(out_color.a == 0)
        discard;
}
