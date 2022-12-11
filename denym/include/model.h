#ifndef _model_h_
#define _model_h_


#include "denym_common.h"


renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, uint32_t instancesCount, uint32_t useIndices, uint32_t useTexCoords, uint32_t useNormals);


#endif
