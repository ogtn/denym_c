#include "primitives.h"

#include <string.h>


static void createCubeData(float size, uint32_t subdivisions, vec3f **positions, vec2f **texCoords, vec3f **normals, uint32_t *vertexCount);

static void createFace(float size, uint32_t subdivisions, uint32_t *axes, float dir, vec3f *positions, vec3f *normals, vec2f *texCoords, const uint32_t *coordBounds, const float *posBounds);


renderable primitiveCreateGrid(float size, uint32_t level)
{
    static const float black[] = {0, 0, 0};
    static const float grey[] = {0.2f, 0.2f, 0.2f};
    const float end = size / 2.f;
    const float start = -end;

    uint32_t subdivisions = 1 << level;
    uint32_t vertexCount = (subdivisions + 1) * 4;
    float *positions = malloc(sizeof(float) * 2 * vertexCount);
    float *colors = malloc(sizeof(float) * 3 * vertexCount);
    float step = size / (float)subdivisions;
    float *pos = positions;
    float *col = colors;

    for(uint32_t i = 0; i <= subdivisions; i++)
    {
        float current = start + (float)i * step;

        // x axis
        *pos++ = current;
        *pos++ = start;
        *pos++ = current;
        *pos++ = end;

        // y axis
        *pos++ = start;
        *pos++ = current;
        *pos++ = end;
        *pos++ = current;

        for(uint32_t j = 0; j < 4; j++)
        {
            memcpy(col, i == subdivisions / 2 ? black : grey, sizeof black);
            col += 3;
        }
    }

    geometryParams geometryParams = geometryCreateParameters(vertexCount, 0);
    geometryParamsAddAttribVec2(geometryParams, positions);
    geometryParamsAddAttribVec3(geometryParams, colors);

    renderableCreateParams renderableParams = {
        .vertShaderName = "grid.vert.spv",
        .fragShaderName = "grid.frag.spv",
        .useWireFrame = 1,
        .geometry = geometryCreate(geometryParams),
        .sendMVP = 1,
        .compactMVP = 1
    };

    free(positions);
    free(colors);

    return renderableCreate(&renderableParams, 1);
}


static void createFace(float size, uint32_t subdivisions, uint32_t *axes, float dir, vec3f *positions, vec3f *normals, vec2f *texCoords, const uint32_t *coordBounds, const float *posBounds)
{
    // normals
    vec3f norm = { 0 };
    norm.v[axes[2]] = dir;

    for(uint32_t i = 0; i < 6 * subdivisions * subdivisions; i++)
    {
        normals[i] = norm;
        positions[i].v[axes[2]] = dir * size / 2;
    }

    for(uint32_t i = 0; i < subdivisions; i++)
    {
        float start_ratio_i = 1 / (float)subdivisions * (float)i;
        float end_ratio_i = 1 / (float)subdivisions * (float)(i + 1);

        float start_p_1 = size / 2 * posBounds[0] + size * start_ratio_i * -posBounds[0];
        float end_p_1 = size / 2 * posBounds[0] + size * end_ratio_i * -posBounds[0];
        float start_tc_1 = coordBounds[0] ? 1 - start_ratio_i : start_ratio_i;
        float end_tc_1 = coordBounds[0] ? 1 - end_ratio_i : end_ratio_i;

        for(uint32_t j = 0; j < subdivisions; j++)
        {
            float start_ratio_j = 1 / (float)subdivisions * (float)j;
            float end_ratio_j = 1 / (float)subdivisions * (float)(j + 1);

            float start_2 = size / 2 * posBounds[1] + size * start_ratio_j * -posBounds[1];
            float end_2 = size / 2 * posBounds[1] + size * end_ratio_j * -posBounds[1];
            float start_tc_2 = coordBounds[1] ? 1 - start_ratio_j : start_ratio_j;
            float end_tc_2 = coordBounds[1] ? 1 - end_ratio_j : end_ratio_j;

            positions[0].v[axes[0]] = positions[2].v[axes[0]] = positions[3].v[axes[0]] = start_p_1;
            positions[1].v[axes[0]] = positions[4].v[axes[0]] = positions[5].v[axes[0]] = end_p_1;
            positions[0].v[axes[1]] = positions[1].v[axes[1]] = positions[4].v[axes[1]] = start_2;
            positions[2].v[axes[1]] = positions[3].v[axes[1]] = positions[5].v[axes[1]] = end_2;

            texCoords[0].u = texCoords[2].u = texCoords[3].u = start_tc_1;
            texCoords[1].u = texCoords[4].u = texCoords[5].u = end_tc_1;
            texCoords[0].v = texCoords[1].v = texCoords[4].v = start_tc_2;
            texCoords[2].v = texCoords[3].v = texCoords[5].v = end_tc_2;

            positions += 6;
            texCoords += 6;
        }
    }
}


static void createCubeData(float size, uint32_t subdivisions, vec3f **positions, vec2f **texCoords, vec3f **normals, uint32_t *vertexCount)
{
    *vertexCount = 6 * 6 * subdivisions * subdivisions;
    *positions = malloc(*vertexCount * sizeof **positions);
    *texCoords = malloc(*vertexCount * sizeof **texCoords);
    *normals = malloc(*vertexCount * sizeof **normals);

    uint32_t coordBounds[6][2] =
    {
        { 1, 0 },
        { 0, 1 },
        { 1, 0 },
        { 1, 0 },
        { 0, 1 },
        { 1, 0 }
    };

    float posBounds[2][2] =
    {
        { -1, -1 },
        { 1, -1 }
    };

    uint32_t axes[3][3] =
    {
        { 2, 1, 0 },
        { 0, 2, 1 },
        { 0, 1, 2 },
    };

    ptrdiff_t offset = 6 * subdivisions * subdivisions;
    createFace(size, subdivisions, axes[0], 1, *positions, *normals, *texCoords, coordBounds[0], posBounds[1]);
    createFace(size, subdivisions, axes[1], 1, *positions + offset, *normals + offset, *texCoords + offset, coordBounds[1], posBounds[1]);
    createFace(size, subdivisions, axes[2], 1, *positions + offset * 2, *normals + offset * 2, *texCoords + offset * 2, coordBounds[2], posBounds[0]);
    createFace(size, subdivisions, axes[0], -1, *positions + offset * 3, *normals + offset * 3, *texCoords + offset * 3, coordBounds[3], posBounds[0]);
    createFace(size, subdivisions, axes[1], -1, *positions + offset * 4, *normals + offset * 4, *texCoords + offset * 4, coordBounds[4], posBounds[0]);
    createFace(size, subdivisions, axes[2], -1, *positions + offset * 5, *normals + offset * 5, *texCoords + offset * 5, coordBounds[5], posBounds[1]);
}


renderable primitiveCreateCube(float size, uint32_t subdivisions, renderableCreateParams *params, uint32_t instancesCount)
{
    vec3f *positions;
    vec2f *texCoords;
    vec3f *normals;
    uint32_t vertexCount;

    createCubeData(size, subdivisions, &positions, &texCoords, &normals, &vertexCount);

    geometryParams geometryParams = geometryCreateParameters(vertexCount, 0);
    geometryParamsAddAttribVec3(geometryParams, (void*)positions);
    geometryParamsAddAttribVec2(geometryParams, (void*)texCoords);
    geometryParamsAddAttribVec3(geometryParams, (void*)normals);

    params->geometry = geometryCreate(geometryParams);

    free(positions);
    free(texCoords);
    free(normals);

    return renderableCreate(params, instancesCount);
}


renderable primitiveCreateSphere(float radius, uint32_t subdivisions, renderableCreateParams *params, uint32_t instancesCount)
{
    vec3f *positions;
    vec2f *texCoords;
    vec3f *normals;
    uint32_t vertexCount;

    createCubeData(radius, subdivisions, &positions, &texCoords, &normals, &vertexCount);

    for(uint32_t i = 0; i < vertexCount; i++)
    {
        glm_vec3_scale(positions[i].v, radius / glm_vec3_norm(positions[i].v) * 0.5f, positions[i].v);
        normals[i] = positions[i];
        glm_vec3_normalize(normals[i].v);
    }

    geometryParams geometryParams = geometryCreateParameters(vertexCount, 0);
    geometryParamsAddAttribVec3(geometryParams, (void*)positions);
    geometryParamsAddAttribVec2(geometryParams, (void*)texCoords);
    geometryParamsAddAttribVec3(geometryParams, (void*)normals);

    params->geometry = geometryCreate(geometryParams);

    free(positions);
    free(texCoords);
    free(normals);

    return renderableCreate(params, instancesCount);
}
