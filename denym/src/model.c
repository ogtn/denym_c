#include "model.h"
#include "logger.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <fast_obj.h>
#include <stb_ds.h>

#pragma clang diagnostic pop


typedef struct pos_t
{
    float x;
    float y;
    float z;
} pos_t;


typedef struct txc_t
{
    float u;
    float v;
} txc_t;


typedef pos_t norm_t;


typedef struct vertexData
{
    uint32_t triangleCount;
    uint32_t indexCount;
    pos_t *positions;
    txc_t *texCoords;
    norm_t *normals;
} vertexData;


typedef struct vertex_vt
{
    pos_t position;
    txc_t texCoord;
    norm_t normal;
} vertex_vt;


static int modelReadOBJ(const char *objFile, vertexData *vertexData);

static geometry modelLoadInternal(const char *objFile, int useIndices, int useNormals);

static void modelAddVertex(uint32_t indexSrc, uint32_t indexDst, fastObjMesh *mesh, vertexData *vertexData);


static int modelReadOBJ(const char *objFile, vertexData *vertexData)
{
    // TODO: check extension
    char fullName[FILENAME_MAX];
	snprintf(fullName, FILENAME_MAX, "resources/models/%s", objFile);
    fastObjMesh* mesh = fast_obj_read(fullName);

    if(mesh == NULL)
        return -1;

    vertexData->triangleCount = 0;

    for(uint32_t i = 0; i < mesh->face_count; i++)
        vertexData->triangleCount += mesh->face_vertices[i] - 2;

    vertexData->positions = malloc(sizeof *vertexData->positions * vertexData->triangleCount * 3);
    vertexData->texCoords = malloc(sizeof *vertexData->texCoords * vertexData->triangleCount * 3);
    vertexData->normals = malloc(sizeof *vertexData->normals * vertexData->triangleCount * 3);

    uint32_t objVertIndex = 0;
    vertexData->indexCount = 0;

    for(uint32_t i = 0; i < mesh->face_count; i++)
    {
        uint32_t baseVertex = objVertIndex;
        uint32_t nextVertex = objVertIndex + 1;

        for(uint32_t k = 0; k < mesh->face_vertices[i] - 2; k++)
        {
            modelAddVertex(baseVertex, vertexData->indexCount++, mesh, vertexData);

            for(uint32_t j = 0; j < 2; j++)
                modelAddVertex(nextVertex++, vertexData->indexCount++, mesh, vertexData);

            nextVertex--;
        }

        objVertIndex += mesh->face_vertices[i];
    }

    fast_obj_destroy(mesh);

    return 0;
}


static geometry modelLoadInternal(const char *objFile, int useIndices, int useNormals)
{
    vertexData vertexData;
    modelReadOBJ(objFile, &vertexData);

    geometry geometry = NULL;

    if(useIndices)
    {
        float start = getUptime();
        uint32_t vertexCount = 0;
        uint32_t *indices32 = malloc(sizeof *indices32 * vertexData.indexCount);
        uint16_t *indices16 = NULL;
        pos_t *newPositions = malloc(sizeof *newPositions * vertexData.indexCount);
        txc_t *newTexCoords = malloc(sizeof *newTexCoords * vertexData.indexCount);
        norm_t *newNormals = malloc(sizeof *newNormals * vertexData.indexCount);

        struct { vertex_vt key; uint32_t value; } *hash = NULL;

        for(uint32_t i = 0; i < vertexData.indexCount; i++)
        {
            vertex_vt vertex = {
                .position = vertexData.positions[i],
                .texCoord = vertexData.texCoords[i],
            };

            if(useNormals)
                vertex.normal = vertexData.normals[i];

            uint32_t currentVertex = (uint32_t)hmgeti(hash, vertex);

            if(currentVertex == UINT32_MAX)
            {
                currentVertex = vertexCount++;
                hmput(hash, vertex, currentVertex);

                newPositions[currentVertex] = vertex.position;
                newTexCoords[currentVertex] = vertex.texCoord;

                if(useNormals)
                {
                    newNormals[currentVertex] = vertex.normal;
                }
                else
                {
                    newNormals[currentVertex].x = 0;
                    newNormals[currentVertex].y = 0;
                    newNormals[currentVertex].z = 0;
                }

                if(vertexCount == UINT32_MAX)
                {
                    logError("MAX INDICES reached\n");

                    break;
                }
            }

            indices32[i] = currentVertex;
        }

        size_t initialSize = vertexData.indexCount * sizeof(pos_t) + sizeof(txc_t);
        size_t finalSize = vertexCount * (sizeof(pos_t) + sizeof(txc_t));

        geometryParams geometryParams = geometryCreateParameters(vertexCount, vertexData.indexCount);
        geometryParamsAddPositions3D(geometryParams, (float*)newPositions);
        geometryParamsAddTexCoords(geometryParams, (float*)newTexCoords);

        if(useNormals)
        {
            geometryParamsAddNormals(geometryParams, (float*)newNormals);
            initialSize += vertexData.indexCount * sizeof(norm_t);
            finalSize += vertexCount * sizeof(norm_t);
        }

        // if possible, use 16bits indices
        if(vertexData.indexCount < UINT16_MAX)
        {
            indices16 = malloc(sizeof *indices16 * vertexData.indexCount);

            for(uint32_t i = 0; i < vertexData.indexCount; i++)
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
        free(newNormals);
        hmfree(hash);

        logInfo("fast_obj indexify: %fs", getUptime() - start);
    }
    else
    {
        geometryParams geometryParams = geometryCreateParameters(vertexData.indexCount, 0);
        geometryParamsAddPositions3D(geometryParams, (float*)vertexData.positions);
        geometryParamsAddTexCoords(geometryParams, (float*)vertexData.texCoords);

        if(useNormals)
            geometryParamsAddNormals(geometryParams, (float*)vertexData.normals);

        geometry = geometryCreate(geometryParams);
    }

    free(vertexData.positions);
    free(vertexData.texCoords);
    free(vertexData.normals);

    return geometry;
}


renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, uint32_t instancesCount, int useIndices, int useNormals)
{
    float start = getUptime();
    renderableParams->geometry = modelLoadInternal(objFile, useIndices, useNormals);
    renderable renderable = renderableCreate(renderableParams, instancesCount);
    logInfo("Model loaded in: %fs", getUptime() - start);

    return renderable;
}


static void modelAddVertex(uint32_t indexSrc, uint32_t indexDst, fastObjMesh *mesh, vertexData *vertexData)
{
    uint32_t pos = mesh->indices[indexSrc].p;
    uint32_t txc = mesh->indices[indexSrc].t;
    uint32_t norm = mesh->indices[indexSrc].n;

    memcpy(&vertexData->positions[indexDst], &mesh->positions[pos * 3], sizeof *vertexData->positions);
    vertexData->texCoords[indexDst].u = mesh->texcoords[txc * 2];
    vertexData->texCoords[indexDst].v = -mesh->texcoords[txc * 2 + 1]; // invert Y axis between obj and Vulkan
    memcpy(&vertexData->normals[indexDst], &mesh->normals[norm * 3], sizeof *vertexData->normals);
}
