#ifndef _renderable_h_
#define _renderable_h_

#include "denym_common.h"


#define RENDERABLE_MAX_PUSH_CONSTANTS 	2
#define RENDERABLE_MAX_UNIFORMS			3
#define RENDERABLE_MAX_BINDINGS 		4


typedef struct renderable_t
{
	geometry geometry;

	// shaders
	shader vertShader;
	shader fragShader;
	VkPipelineShaderStageCreateInfo shaderStages[2];

	// descriptors
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

	// uniforms
	struct
	{
		VkBuffer buffers[RENDERABLE_MAX_UNIFORMS];
		VkDeviceMemory buffersMemory[RENDERABLE_MAX_UNIFORMS];
		uint32_t count;
		VkDeviceSize sizePerFrame[RENDERABLE_MAX_UNIFORMS];
		void *cache[RENDERABLE_MAX_UNIFORMS];
	} uniforms;

	// storage buffer
	VkBuffer storageBuffer;
	VkDeviceMemory storageBufferMemory;
	VkBool32 useStorageBuffer;
	VkDeviceSize storageBufferSizePerFrameAndInstance;
	VkDeviceSize storageBufferSizePerFrame;
	VkDeviceSize storageBufferTotalSize;
	void *storageBufferCache;

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
	VkBool32 sendMVPAsStorageBuffer;
	VkBool32 sendLights;

	uint32_t instanceCount;
	mat4 modelMatrix;
	VkBool32 needMVPUpdate[MAX_FRAMES_IN_FLIGHT];
} renderable_t;


renderable renderableCreate(const renderableCreateParams *params, uint32_t instanceCount);

void renderableDestroy(renderable renderable);

int renderableLoadShaders(renderable renderable, const char *vertShaderName, const char *fragShaderName);

int renderableCreatePipelineLayout(renderable renderable);

int renderableCreatePipeline(renderable renderable);

void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer);

uint32_t renderableAddUniformInternal(renderable renderable, VkDeviceSize size);

int renderableCreateUniformsBuffers(renderable renderable);

int renderableCreateStorageBuffer(renderable renderable);

int renderableUpdateUniformsBuffer(renderable renderable, uint32_t id, const void *data);

int renderableUpdateStorageBuffer(renderable renderable, const void *data, uint32_t instanceId);

int renderableCreateDescriptorPool(renderable renderable);

int renderableCreateDescriptorSetLayout(renderable renderable);

int renderableCreateDescriptorSets(renderable renderable, VkBool32 useNearestSampler);

int renderableUpdatePushConstant(renderable renderable, void *value);

int renderableUpdatePushConstantInternal(renderable renderable, void *value, uint32_t pushConstantNumber);

void renderableSetMatrix(renderable renderable, mat4 matrix);

void renderableUpdateMVP(renderable renderable, VkBool32 force);

void renderableUpdateLighting(renderable renderable);

void renderableSetMatrixInstance(renderable renderable, mat4 matrix, uint32_t instanceId);

int renderableAddPushConstant(renderable renderable, uint32_t size, VkShaderStageFlags shaderStage);

#endif
