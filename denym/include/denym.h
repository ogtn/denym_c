#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(void);

void denymWaitForNextFrame(void);


typedef struct geometry_t* geometry;

geometry denymCreateGeometry(uint32_t vertexCount);

typedef struct renderable_t* renderable;

renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName);


#endif
