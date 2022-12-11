#ifndef _primitives_h_
#define _primitives_h_


#include "denym_common.h"


renderable primitiveCreateGrid(float size, uint32_t level);

renderable primitiveCreateCube(float size, uint32_t subdivisions, renderableCreateParams *params, uint32_t instanceCount, uint32_t useTexCoords, uint32_t useNormals);

renderable primitiveCreateSphere(float radius, uint32_t subdivisions, renderableCreateParams *params, uint32_t instanceCount, uint32_t useTexCoords, uint32_t useNormals);


#endif
