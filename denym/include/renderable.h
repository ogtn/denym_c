#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"
#include "geometry.h"


typedef struct renderable_t
{
	geometry geometry;
	const char *vertShaderName;
	const char *fragShaderName;
	VkCommandBuffer* commandBuffers; // draw cmds
	VkPipelineLayout pipelineLayout; // uniforms sent to shaders
	VkPipeline pipeline;             // type of render
} renderable_t;


int createPipeline(renderable renderable);


#endif
