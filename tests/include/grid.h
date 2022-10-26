#include "denym.h"

#include <string.h>


static renderable createGrid(float size, uint32_t level)
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
        .sendMVP = 1
    };

    free(positions);
    free(colors);

    return renderableCreate(&renderableParams);
}
