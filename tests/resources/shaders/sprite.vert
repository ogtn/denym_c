#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 mvp;
} ubo;

layout(push_constant) uniform PCST
{
    vec2 start;
    vec2 end;
} pcst;

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec2 out_texCoord;

void main()
{
    vec2 coordinates[6] = {
        { pcst.start.s, pcst.end.t},
        { pcst.end.s,   pcst.end.t},
        { pcst.start.s, pcst.start.t},

        { pcst.end.s,   pcst.end.t},
        { pcst.end.s,   pcst.start.t},
        { pcst.start.s, pcst.start.t}
    };

    gl_Position = ubo.mvp * vec4(in_position, 1);
    out_texCoord = coordinates[gl_VertexIndex];
}
