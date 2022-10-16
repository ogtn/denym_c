#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"


typedef struct renderable_t
{
	geometry geometry;

	// shaders
	char vertShaderName[FILENAME_MAX];
	char fragShaderName[FILENAME_MAX];
	VkShaderModule vertShader;
	VkShaderModule fragShader;
	VkPipelineShaderStageCreateInfo shaderStages[2];

	// descriptors
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

	// uniforms
	VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
	VkBool32 useUniforms;
	VkDeviceSize uniformSize;

	// push constant
	VkBool32 usePushConstant;
	float pushConstantAlpha;

	// pipeline
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	// texture
	texture texture;
	VkBool32 useTexture;

	VkPolygonMode polygonMode;
	VkPrimitiveTopology primitiveTopology;
} renderable_t;


renderable denymCreateRenderable(const renderableCreateParams *params);

void denymDestroyRenderable(renderable renderable);

int updateUniformsBuffer(renderable renderable, const void *data);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);

int loadShaders(renderable renderable);

int createPipelineLayout(renderable renderable);

int createPipeline(renderable renderable);

void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer);

int createUniformsBuffer(renderable renderable);

int useUniforms(renderable renderable);

int createDescriptorPool(renderable renderable);

int createDescriptorSetLayout(renderable renderable);

int createDescriptorSets(renderable renderable);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);


#endif
