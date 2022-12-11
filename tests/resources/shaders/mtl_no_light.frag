#version 450
#extension GL_ARB_separate_shader_objects : enable

struct material_t
{
    vec3 color;
    float shininess;
};

layout(binding = 0) uniform MATERIAL
{
    material_t material;
} material;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(material.material.color, 1);

    if(outColor.a == 0)
        discard;
}
