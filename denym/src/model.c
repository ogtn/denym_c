#include "model.h"
#include "logger.h"


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


static void addVertex(uint32_t indexSrc, uint32_t indexDst, fastObjIndex *indices, float *posSrc, float *posDst, float *texSrc, float *texDst, float *normSrc, float *normDst);


renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, int useIndices, int useNormals)
{
    // TODO: check extension
    float start = getUptime();
    char fullName[FILENAME_MAX];
	snprintf(fullName, FILENAME_MAX, "resources/models/%s", objFile);
    fastObjMesh* mesh = fast_obj_read(fullName);
    logInfo("fast_obj_read: %fs", getUptime() - start);
    start = getUptime();

    uint32_t triangleCount = 0;

    for(uint32_t i = 0; i < mesh->face_count; i++)
        triangleCount += mesh->face_vertices[i] - 2;

    float *positions = malloc(sizeof(float) * 3 * triangleCount * 3);
    float *texCoords = malloc(sizeof(float) * 2 * triangleCount * 3);
    float *normals = malloc(sizeof(float) * 3 * triangleCount * 3);

    uint32_t objVertIndex = 0;
    uint32_t index = 0;

    for(uint32_t i = 0; i < mesh->face_count; i++)
    {
        uint32_t baseVertex = objVertIndex;
        uint32_t nextVertex = objVertIndex + 1;

        for(uint32_t k = 0; k < mesh->face_vertices[i] - 2; k++)
        {
            addVertex(baseVertex, index++, mesh->indices,
                mesh->positions, positions,
                mesh->texcoords, texCoords,
                mesh->normals, normals);

            for(uint32_t j = 0; j < 2; j++)
            {
                addVertex(nextVertex++, index++, mesh->indices,
                    mesh->positions, positions,
                    mesh->texcoords, texCoords,
                    mesh->normals, normals);
            }

            nextVertex--;
        }

        objVertIndex += mesh->face_vertices[i];
    }

    logInfo("fast_obj transform: %fs", getUptime() - start);

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
                    logError("MAX INDICES reached\n");

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
                indices16[i] = (uint16_t)indices32[i];

            geometryParamsAddIndices16(geometryParams, indices16);
            finalSize += vertexCount * sizeof *indices16;
        }
        else
        {
            geometryParamsAddIndices32(geometryParams, indices32);
            finalSize += vertexCount * sizeof *indices32;
        }

        double gain = (double)(initialSize - finalSize) / (double)initialSize * 100;
        logInfo("Initial size : %ld, indexified size  : %ld. Gain : %.2lf%%",
            initialSize, finalSize, gain);

        geometry = geometryCreate(geometryParams);

        free(indices32);
        free(indices16);
        free(newPositions);
        free(newTexCoords);
        hmfree(hash);

        logInfo("fast_obj indexify: %fs", getUptime() - start);
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

    renderableParams->geometry = geometry;
    renderable renderable = renderableCreate(renderableParams);

    free(positions);
    free(texCoords);
    free(normals);
    fast_obj_destroy(mesh);

    return renderable;
}


static void addVertex(uint32_t indexSrc, uint32_t indexDst, fastObjIndex *indices, float *posSrc, float *posDst, float *texSrc, float *texDst, float *normSrc, float *normDst)
{
    uint32_t pos = indices[indexSrc].p;
    uint32_t txc = indices[indexSrc].t;
    uint32_t norm = indices[indexSrc].n;

    memcpy(&posDst[indexDst * 3], &posSrc[pos * 3], sizeof(vec3));
    texDst[indexDst * 2] = texSrc[txc * 2];
    texDst[indexDst * 2 + 1] = -texSrc[txc * 2 + 1]; // invert Y axis between obj and Vulkan
    memcpy(&normDst[indexDst * 3], &normSrc[norm * 3], sizeof(vec3));
}
