#ifndef _denym_h_
#define _denym_h_


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(void);

void denymWaitForNextFrame(void);


#endif
