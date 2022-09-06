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
	VkDescriptorSet uniformDescriptorSets[MAX_FRAMES_IN_FLIGHT];
	VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
	VkBool32 useUniforms;

	// push constant
	VkBool32 usePushConstant;
	float pushConstantAlpha;

	// state
	VkBool32 isReady;

	// pipeline
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline; // type of render
} renderable_t;


renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName);

int makeReady(renderable renderable);

void denymDestroyRenderable(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);

int createPipeline(renderable renderable);

void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer);

int createUniformsBuffer(renderable renderable);

int useUniforms(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int createDescriptorPool(renderable renderable);

int createDescriptorSetLayout(renderable renderable);

int createDescriptorSets(renderable renderable);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);


#endif
