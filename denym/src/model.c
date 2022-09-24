#include "model.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <fast_obj.h>
#include <stb_ds.h>

#pragma clang diagnostic pop


typedef struct vertex_vt
{
    struct { float x; float y; float z; } position;
    struct { float u; float v; } texCoord;
} vertex_vt;


renderable modelLoad(const char *objFile, int indexify,  const char *texture, const char *vertShader, const char *fragShader)
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
        // TODO: generate triangles from n-gons
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

    geometry geometry = NULL;

    if(indexify)
    {
        start = getUptime();
        uint32_t vertexCount = 0;
        uint16_t *indices = malloc(sizeof *indices * index);
        float *newPositions = malloc(sizeof(float) * 3 * index);
        float *newTexCoords = malloc(sizeof(float) * 2 * index);

        struct { vertex_vt key; uint16_t value; } *hash = NULL;

        for(uint32_t i = 0; i < index; i++)
        {
            vertex_vt vertex = {
                .position.x = positions[i * 3],
                .position.y = positions[i * 3 + 1],
                .position.z = positions[i * 3 + 2],

                .texCoord.u = texCoords[i * 2],
                .texCoord.v = texCoords[i * 2 + 1]
            };

            uint16_t currentVertex = (uint16_t)hmgeti(hash, vertex);

            if(currentVertex == UINT16_MAX)
            {
                currentVertex = vertexCount++;
                hmput(hash, vertex, currentVertex);

                newPositions[currentVertex * 3] = vertex.position.x;
                newPositions[currentVertex * 3 + 1] = vertex.position.y;
                newPositions[currentVertex * 3 + 2] = vertex.position.z;

                newTexCoords[currentVertex * 2] = vertex.texCoord.u;
                newTexCoords[currentVertex * 2 + 1] = vertex.texCoord.v;

                if(vertexCount == UINT16_MAX)
                {
                    fprintf(stderr, "MAX INDICES reached\n");

                    break;
                }
            }

            indices[i] = currentVertex;
        }

        geometryCreateParams geometryParams = {
            .positions3D = newPositions,
            .texCoords = newTexCoords,
            .indexCount = index,
            .indices = indices,
            .vertexCount = vertexCount
        };

        geometry = geometryCreate(&geometryParams);
        free(indices);
        free(newPositions);
        free(newTexCoords);
        hmfree(hash);

        printf("fast_obj indexify: %fs\n", getUptime() - start);
    }
    else
    {
        geometryCreateParams geometryParams = {
            .positions3D = positions,
            .texCoords = texCoords,
            .vertexCount = index
        };

        geometry = geometryCreate(&geometryParams);
    }

    renderableCreateParams renderableParams = {
        .fragShaderName = fragShader,
        .vertShaderName = vertShader,
        .textureName = texture,
        .geometry = geometry,
        .useUniforms = 1
    };
    renderable renderable = denymCreateRenderable(&renderableParams);

    free(positions);
    free(texCoords);

    return renderable;
}
