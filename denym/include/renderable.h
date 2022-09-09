#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"


typedef struct renderable_t
{
	geometry geometry;

	// shaders
	const char *vertShaderName;
	const char *fragShaderName;
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipelineShaderStageCreateInfo shaderStages[2];

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
	VkPipeline pipeline;

	// texture
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
} renderable_t;


renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName);

int makeReady(renderable renderable);

void denymDestroyRenderable(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);

int loadShaders(renderable renderable);

int createPipelineLayout(renderable renderable);

int createPipeline(renderable renderable);

int recreatePipeline(renderable renderable);

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
