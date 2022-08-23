#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(void);

void denymWaitForNextFrame(void);

int denymCreateGeometry(uint32_t vertexCount, const char *vertShaderName, const char *fragShaderName);


#endif
