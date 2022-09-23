#include "model.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

#pragma clang diagnostic pop


renderable modelLoad(const char *objFile, const char *texture, const char *vertShader, const char *fragShader)
{
    // TODO: check extension
    float start = getUptime();
    char fullName[FILENAME_MAX];
	snprintf(fullName, FILENAME_MAX, "resources/models/%s", objFile);
    fastObjMesh* mesh = fast_obj_read(fullName);
    printf("fast_obj_read: %fs\n", getUptime() - start);
    start = getUptime();

    float *positions = malloc(sizeof(float) * 3 * mesh->index_count);
    float *texCoords = malloc(sizeof(float) * 2 * mesh->index_count);

    uint32_t index_f = 0;
    uint32_t index = 0;

    for(uint32_t i = 0; i < mesh->face_count; i++)
    {
        if(mesh->face_vertices[i] == 3)
        {
            for(uint32_t j = 0; j < mesh->face_vertices[i]; j++)
            {
                uint32_t pos = mesh->indices[index_f].p;
                uint32_t txc = mesh->indices[index_f].t;

                positions[index * 3] = mesh->positions[pos * 3];
                positions[index * 3 + 1] = mesh->positions[pos * 3 + 1];
                positions[index * 3 + 2] = mesh->positions[pos * 3 + 2];

                texCoords[index * 2] = mesh->texcoords[txc * 2];
                texCoords[index * 2 + 1] = 1 - mesh->texcoords[txc * 2 + 1];

                index_f++;
                index++;
            }
        }
        else
            index_f += mesh->face_vertices[i];
    }

    printf("fast_obj transform: %fs\n", getUptime() - start);

    geometryCreateParams geometryParams = {
        .positions3D = positions,
        .texCoords = texCoords,
        .vertexCount = index
    };

    renderableCreateParams renderableParams = {
        .fragShaderName = fragShader,
        .vertShaderName = vertShader,
        .textureName = texture,
        .geometry = geometryCreate(&geometryParams),
        .useUniforms = 1
    };
    renderable renderable = denymCreateRenderable(&renderableParams);

    free(positions);
    free(texCoords);

    return renderable;
}
