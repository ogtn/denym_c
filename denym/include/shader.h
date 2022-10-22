#ifndef _shader_h_
#define _shader_h_


#include "denym_common.h"


typedef struct shader_t
{
    char name[FILENAME_MAX];
    VkShaderModule shaderModule;
} shader_t;


shader shaderCreate(VkDevice device, const char* name);

void shaderDestroy(shader shader);


#endif
