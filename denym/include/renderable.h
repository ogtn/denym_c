#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"


typedef struct renderable_t
{
	geometry geometry;

	// shaders
	char vertShaderName[FILENAME_MAX];
	char fragShaderName[FILENAME_MAX];
	shader vertShader;
	shader fragShader;
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
	void *uniformCache[MAX_FRAMES_IN_FLIGHT];

	// push constant
	VkBool32 usePushConstant;
	uint32_t pushConstantSize;
	void *pushConstantValue;

	// pipeline
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	// texture
	texture texture;
	VkBool32 useTexture;

	// type of geometry
	VkPolygonMode polygonMode;
	VkPrimitiveTopology primitiveTopology;

	VkBool32 compactMVP;
	mat4 modelMatrix;
} renderable_t;


renderable renderableCreate(const renderableCreateParams *params);

void renderableDestroy(renderable renderable);

int renderableLoadShaders(renderable renderable);

int renderableCreatePipelineLayout(renderable renderable);

int renderableCreatePipeline(renderable renderable);

void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer);

int renderableCreateUniformsBuffer(renderable renderable);

int renderableUpdateUniformsBuffer(renderable renderable, const void *data);

int renderableCreateDescriptorPool(renderable renderable);

int renderableCreateDescriptorSetLayout(renderable renderable);

int renderableCreateDescriptorSets(renderable renderable);

int renderableUpdatePushConstant(renderable renderable, void *value);

void renderableSetMatrix(renderable renderable, mat4 matrix);


#endif
