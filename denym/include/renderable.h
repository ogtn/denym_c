#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"
#include "geometry.h"


typedef struct renderable_t
{
	geometry geometry;
	const char *vertShaderName;
	const char *fragShaderName;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkCommandBuffer* commandBuffers; // draw cmds
	VkPipelineLayout pipelineLayout; // uniforms sent to shaders, amongst other things
	VkPipeline pipeline;             // type of render

	VkBool32 needCommandBufferUpdate;
} renderable_t;


int createPipeline(renderable renderable);

int createCommandBuffers(renderable renderable);

int updateCommandBuffers(renderable renderable);

#endif
