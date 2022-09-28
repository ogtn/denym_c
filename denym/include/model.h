#ifndef _model_h_
#define _model_h_


#include "denym_common.h"


renderable modelLoad(const char *objFile, int useIndices, int useNormals, const char *texture, const char *vertShader, const char *fragShader);


#endif
