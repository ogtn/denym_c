#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"


#define RENDERABLE_MAX_PUSH_CONSTANTS 2


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
	struct
	{
		uint32_t totalSize;
		uint32_t count;
		VkShaderStageFlags shaderStages[RENDERABLE_MAX_PUSH_CONSTANTS];
		uint32_t sizes[RENDERABLE_MAX_PUSH_CONSTANTS];
		void *values[RENDERABLE_MAX_PUSH_CONSTANTS];
	} pushConstants;

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
	VkBool32 sendMVPAsPushConstant;
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

int renderableUpdatePushConstantInternal(renderable renderable, void *value, uint32_t pushConstantNumber);

void renderableSetMatrix(renderable renderable, mat4 matrix);

int renderableAddPushConstant(renderable renderable, uint32_t size, VkShaderStageFlags shaderStage);

#endif
