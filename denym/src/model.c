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
    struct { float x; float y; float z; } normal;
} vertex_vt;


renderable modelLoad(const char *objFile, int useIndices, int useNormals, const char *texture, const char *vertShader, const char *fragShader)
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
    float *normals = malloc(sizeof(float) * 3 * mesh->index_count);

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
                uint32_t norm = mesh->indices[index_f].n;

                positions[index * 3] = mesh->positions[pos * 3];
                positions[index * 3 + 1] = mesh->positions[pos * 3 + 1];
                positions[index * 3 + 2] = mesh->positions[pos * 3 + 2];

                texCoords[index * 2] = mesh->texcoords[txc * 2];
                texCoords[index * 2 + 1] = 1 - mesh->texcoords[txc * 2 + 1];

                normals[index * 3] = mesh->normals[norm * 3];
                normals[index * 3 + 1] = mesh->normals[norm * 3 + 1];
                normals[index * 3 + 2] = mesh->normals[norm * 3 + 2];

                index_f++;
                index++;
            }
        }
        else
            index_f += mesh->face_vertices[i];
    }

    printf("fast_obj transform: %fs\n", getUptime() - start);

    geometry geometry = NULL;

    if(useIndices)
    {
        start = getUptime();
        uint32_t vertexCount = 0;
        uint32_t *indices32 = malloc(sizeof *indices32 * index);
        uint16_t *indices16 = NULL;
        float *newPositions = malloc(sizeof(float) * 3 * index);
        float *newTexCoords = malloc(sizeof(float) * 2 * index);
        float *newNormals = malloc(sizeof(float) * 3 * index);

        struct { vertex_vt key; uint32_t value; } *hash = NULL;

        for(uint32_t i = 0; i < index; i++)
        {
            vertex_vt vertex = {
                .position.x = positions[i * 3],
                .position.y = positions[i * 3 + 1],
                .position.z = positions[i * 3 + 2],

                .texCoord.u = texCoords[i * 2],
                .texCoord.v = texCoords[i * 2 + 1]
            };

            if(useNormals)
            {
                vertex.normal.x = normals[i * 3];
                vertex.normal.y = normals[i * 3 + 1];
                vertex.normal.z = normals[i * 3 + 2];
            }

            uint32_t currentVertex = (uint32_t)hmgeti(hash, vertex);

            if(currentVertex == UINT32_MAX)
            {
                currentVertex = vertexCount++;
                hmput(hash, vertex, currentVertex);

                newPositions[currentVertex * 3] = vertex.position.x;
                newPositions[currentVertex * 3 + 1] = vertex.position.y;
                newPositions[currentVertex * 3 + 2] = vertex.position.z;

                newTexCoords[currentVertex * 2] = vertex.texCoord.u;
                newTexCoords[currentVertex * 2 + 1] = vertex.texCoord.v;

                if(useNormals)
                {
                    newNormals[currentVertex * 3] = vertex.normal.x;
                    newNormals[currentVertex * 3 + 1] = vertex.normal.y;
                    newNormals[currentVertex * 3 + 2] = vertex.normal.z;
                }
                else
                {
                    newNormals[currentVertex * 3] = 0;
                    newNormals[currentVertex * 3 + 1] = 0;
                    newNormals[currentVertex * 3 + 2] = 0;
                }

                if(vertexCount == UINT32_MAX)
                {
                    fprintf(stderr, "MAX INDICES reached\n");

                    break;
                }
            }

            indices32[i] = currentVertex;
        }

        size_t initialSize = 0;
        size_t finalSize = 0;

        geometryParams geometryParams = geometryCreateParameters(vertexCount, index);
        geometryParamsAddPositions3D(geometryParams, newPositions);
        geometryParamsAddTexCoords(geometryParams, newTexCoords);

        if(useNormals)
        {
            geometryParamsAddNormals(geometryParams, newNormals);
            initialSize += index * sizeof(float) * 3;
            finalSize += vertexCount * sizeof(float) * 3;
        }

        initialSize += index * (sizeof(float) * 3 + sizeof(float) * 2);
        finalSize += vertexCount * (sizeof(float) * 3 + sizeof(float) * 2);

        // if possible, use 16bits indices
        if(index < UINT16_MAX)
        {
            indices16 = malloc(sizeof *indices16 * index);

            for(uint32_t i = 0; i < index; i++)
                indices16[i] = indices32[i];

            geometryParamsAddIndices16(geometryParams, indices16);
            finalSize += vertexCount * sizeof *indices16;
        }
        else
        {
            geometryParamsAddIndices32(geometryParams, indices32);
            finalSize += vertexCount * sizeof *indices32;
        }

        double gain = (double)(initialSize - finalSize) / initialSize * 100;
        printf("Initial size : %ld, indexified size  : %ld. Gain : %.2lf%%\n",
            initialSize, finalSize, gain);

        geometry = geometryCreate(geometryParams);

        free(indices32);
        free(indices16);
        free(newPositions);
        free(newTexCoords);
        hmfree(hash);

        printf("fast_obj indexify: %fs\n", getUptime() - start);
    }
    else
    {
        geometryParams geometryParams = geometryCreateParameters(index, 0);
        geometryParamsAddPositions3D(geometryParams, positions);
        geometryParamsAddTexCoords(geometryParams, texCoords);

        if(useNormals)
            geometryParamsAddNormals(geometryParams, normals);

        geometry = geometryCreate(geometryParams);
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
    free(normals);
    fast_obj_destroy(mesh);

    return renderable;
}
