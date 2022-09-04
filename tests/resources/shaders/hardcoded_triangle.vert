#version 450
#extension GL_ARB_separate_shader_objects : enable

// awful but ok for a first test, positions and colors are embeded in the shader
vec3 colors[3] = vec3[]
(
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1)
);

// clip coordinates
vec2 positions[3] = vec2[]
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

layout(location = 0) out vec3 fragColor;

void main()
{
    // z = 0, w = 1 so transformation to clip coordinate will be a non op
    gl_Position = vec4(positions[gl_VertexIndex], 0, 1);

    fragColor = colors[gl_VertexIndex];
}
