#include "denym.h"


#include <stdlib.h>
#include <math.h>
#include <string.h>


static int loadOBJ(const char *filename, float **out_positions, float **out_texCoords, float **out_normals, uint32_t *vertexCount)
{
    *out_positions = NULL;
    *out_texCoords = NULL;
    *out_normals = NULL;
    *vertexCount = 0;

    FILE *file;
    char line[1024];
    uint32_t nbPositions = 0;
    uint32_t nbNormals = 0;
    uint32_t nbTexCoords = 0;
    uint32_t nbFaces = 0;
    int currentPosition;
    int currentNormal;
    int currentTexCoord;
    vec3 *positions;
    vec3 *normals;
    vec2 *texCoords;
    vec3 *tmpPositions;
    vec3 *tmpNormals;
    vec2 *tmpTexCoords;

    char fullName[FILENAME_MAX];
	snprintf(fullName, FILENAME_MAX, "resources/models/%s", filename);

    float start = getUptime();
    float last = start;

    #ifdef _MSC_VER
	fopen_s(&file, filename, "rb");
#else
    file = fopen(fullName, "r");
#endif

    if(file == NULL)
    {
        fprintf(stderr, "Unable to open the file: %s\n", fullName);

        return -1;
    }

    // first pass: count occurences
    // TODO: avoid this with realloc() ?
    while(fgets(line, 1024, file))
    {
        // position
        if(strstr(line, "v ") == line)
            nbPositions++;

        // texture coordinate
        else if(strstr(line, "vt ") == line)
            nbTexCoords++;

        // normal
        else if(strstr(line, "vn ") == line)
            nbNormals++;

        // face
        else if(strstr(line, "f ") == line)
        {
            int nbSlashes = 0;
            char *slash = strchr(line, '/');

            while(slash)
            {
                slash = strchr(slash + 1, '/');
                nbSlashes++;
            }

            if(nbSlashes == 6)
                nbFaces++;
            else if(nbSlashes == 8)
                nbFaces += 2;
            else
            {
                fprintf(stderr, "OBJ loader failed, faces are not triangles nor quads\n");

                return -1;
            }
        }
    }

    // indices start at 1 in OBJ files, need an extra empty case at the beginning of the array
    tmpPositions = malloc((nbPositions + 1) * sizeof *tmpPositions);
    tmpTexCoords = malloc((nbTexCoords + 1) * sizeof *tmpTexCoords);
    tmpNormals = malloc((nbNormals + 1) * sizeof *tmpNormals);

    currentPosition = 1;
    currentTexCoord = 1;
    currentNormal = 1;
    rewind(file);

    printf("Load model pass 1/3: %fs\n", getUptime() - last);
    last = getUptime();

    // second pass: load vertex attributes in temporary buffers
    while(fgets(line, 1024, file))
    {
        // TODO: skip better than that?
        // skip comments
        if(*line == '#')
            continue;

        // positions
        if(strstr(line, "v ") == line)
        {
            sscanf(line, "v %f %f %f",
                &tmpPositions[currentPosition][0],
                &tmpPositions[currentPosition][1], // invert Y and Z
                &tmpPositions[currentPosition][2]);

            currentPosition++;
        }

        // texture coordinates
        else if(strstr(line, "vt ") == line)
        {
            sscanf(line, "vt %f %f",
                &tmpTexCoords[currentTexCoord][0],
                &tmpTexCoords[currentTexCoord][1]);

            // different Y in .obj and Vulkan
            tmpTexCoords[currentTexCoord][1] = 1 - tmpTexCoords[currentTexCoord][1];
            currentTexCoord++;
        }

        // normals
        else if(strstr(line, "vn ") == line)
        {
            sscanf(line, "vn %f %f %f",
                &tmpNormals[currentNormal][0],
                &tmpNormals[currentNormal][1],
                &tmpNormals[currentNormal][2]);

            currentNormal++;
        }
    }

    currentPosition = 0;
    currentTexCoord = 0;
    currentNormal = 0;
    rewind(file);

    // buffers allocations
    // TODO: do this after tmp buffers have been filled and compressed to improve performances and memory used?
    positions = malloc(nbFaces * 3 * sizeof *positions);
    texCoords = malloc(nbFaces * 3 * sizeof * texCoords);
    normals = malloc(nbFaces * 3 * sizeof *normals);

    printf("Load model pass 2/3: %fs\n", getUptime() - last);
    last = getUptime();

    // third pass: load faces indices
    // TODO: do this in a temporary buffer during the second pass
    while(fgets(line, 1024, file))
    {
        // TODO: skip better than that?
        // skip comments
        if(*line == '#')
            continue;

        // face
        if(strstr(line, "f ") == line)
        {
            int v1, v2, v3, v4;
            int vt1, vt2, vt3, vt4;
            int vn1, vn2, vn3, vn4;
            int readVal;

            if(strstr(line, "//"))
            {
                readVal = sscanf(line, "f %d//%d %d//%d %d//%d %d//%d",
                &v1, &vn1,
                &v2, &vn2,
                &v3, &vn3,
                &v4, &vn4);
            }
            else
            {
                readVal = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                    &v1, &vt1, &vn1,
                    &v2, &vt2, &vn2,
                    &v3, &vt3, &vn3,
                    &v4, &vt4, &vn4);

                // texture coordinates
                memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt1], sizeof *texCoords);
                memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt2], sizeof *texCoords);
                memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt3], sizeof *texCoords);

                if(readVal == 12)
                {
                    memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt1], sizeof *texCoords);
                    memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt3], sizeof *texCoords);
                    memcpy(&texCoords[currentTexCoord++], &tmpTexCoords[vt4], sizeof *texCoords);
                }
            }

            // positions
            memcpy(&positions[currentPosition++], &tmpPositions[v1], sizeof *positions);
            memcpy(&positions[currentPosition++], &tmpPositions[v2], sizeof *positions);
            memcpy(&positions[currentPosition++], &tmpPositions[v3], sizeof *positions);

            // normals
            memcpy(&normals[currentNormal++], &tmpNormals[vn1], sizeof *normals);
            memcpy(&normals[currentNormal++], &tmpNormals[vn2], sizeof *normals);
            memcpy(&normals[currentNormal++], &tmpNormals[vn3], sizeof *normals);

            if(readVal == 8 || readVal == 12)
            {
                // positions
                memcpy(&positions[currentPosition++], &tmpPositions[v1], sizeof *positions);
                memcpy(&positions[currentPosition++], &tmpPositions[v3], sizeof *positions);
                memcpy(&positions[currentPosition++], &tmpPositions[v4], sizeof *positions);

                // normals
                memcpy(&normals[currentNormal++], &tmpNormals[vn1], sizeof *normals);
                memcpy(&normals[currentNormal++], &tmpNormals[vn3], sizeof *normals);
                memcpy(&normals[currentNormal++], &tmpNormals[vn4], sizeof *normals);
            }
        }
    }

    fclose(file);
    free(tmpPositions);
    free(tmpTexCoords);
    free(tmpNormals);

    *out_positions = (void*)positions;
    *out_texCoords = (void*)texCoords;
    *out_normals = (void*)normals;
    *vertexCount = nbFaces * 3;

    printf("Load model pass 3/3: %fs\n", getUptime() - last);

    return 0;
}


static renderable loadModel(const char *objFile, const char *texture, const char *vertShader, const char *fragShader)
{
    float *positions;
    float *normals;
    float *texCoords;
    uint32_t vertexCount;

    if(loadOBJ(objFile, &positions, &texCoords, &normals, &vertexCount))
        return NULL;

    geometryCreateParams geometryParams = {
        .positions3D = (void*)positions,
        .texCoords = (void*)texCoords,
        .vertexCount = vertexCount
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
    free(normals);

    return renderable;
}


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    renderable model = loadModel(
        "viking_room.obj",
        "viking_room.png",
        "texture_v2.vert.spv",
        "texture_v2.frag.spv");

	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0.5};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), width / height, 0.01f, 1000, mvp.projection);
	mvp.projection[1][1] *= -1;

	while (denymKeepRunning())
	{
        float elapsed_since_start = getUptime();

		glm_mat4_identity(mvp.model);
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 20), axis);
		updateUniformsBuffer(model, &mvp);

		denymRender(&model, 1);
		denymWaitForNextFrame();
	}

    denymDestroyRenderable(model);
	denymTerminate();

	return EXIT_SUCCESS;
}
