#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>


typedef struct geometry_t* geometry;
typedef struct renderable_t* renderable;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(renderable renderable);

void denymWaitForNextFrame(void);

geometry denymCreateGeometry(uint32_t vertexCount);

void denymDestroyGeometry(geometry geometry);

void denymGeometryAddPosition(geometry, float *positions);

void denymGeometryAddColors(geometry, float *colors);

void denymGeometryAddIndices(geometry geometry, uint16_t *indices);

renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName);

void denymDestroyRenderable(renderable renderable);


#endif
