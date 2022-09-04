#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"
#include "geometry.h"


typedef struct renderable_t
{
	geometry geometry;

	// shaders
	// TODO: is this really usefull ? shaderModules are created and destructed in the same function...
	const char *vertShaderName;
	const char *fragShaderName;
	VkShaderModule vertShader;
	VkShaderModule fragShader;

	// uniforms
	VkDescriptorPool uniformDescriptorPool;
	VkDescriptorSetLayout uniformDescriptorSetLayout;
	VkDescriptorSet *uniformDescriptorSets;
	VkBuffer *uniformBuffers;
	VkDeviceMemory *uniformBuffersMemory;

	// command buffers
	VkCommandBuffer* commandBuffers; // draw cmds
	VkBool32 needCommandBufferUpdate;
	int __padding; // TODO: fix this monstruousity

	// pipeline
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline; // type of render
} renderable_t;


int createPipeline(renderable renderable);

int createCommandBuffers(renderable renderable);

int updateCommandBuffers(renderable renderable);

int createUniformsBuffer(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int createDescriptorPool(renderable renderable);

int createDescriptorSetLayout(renderable renderable);

int createDescriptorSets(renderable renderable);

#endif
