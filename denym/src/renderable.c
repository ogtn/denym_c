#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "buffer.h"
#include "core.h"
#include "texture.h"
#include "scene.h"
#include "camera.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MAX_BINDINGS 2


renderable renderableCreate(const renderableCreateParams *params)
{
	renderable renderable = calloc(1, sizeof(*renderable));

	strncpy(renderable->vertShaderName, params->vertShaderName, sizeof renderable->vertShaderName);
	strncpy(renderable->fragShaderName, params->fragShaderName, sizeof renderable->fragShaderName);
	renderable->geometry = params->geometry;
	renderable->useUniforms = params->uniformSize != 0;
	renderable->uniformSize = params->uniformSize;
	renderable->usePushConstant = params->usePushConstant;

	if(params->useWireFrame)
	{
		renderable->polygonMode = VK_POLYGON_MODE_LINE;
		renderable->primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	}
	else
	{
		renderable->polygonMode = VK_POLYGON_MODE_FILL;
		renderable->primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	if(params->textureName)
	{
		if(textureCreate(params->textureName, &renderable->texture))
			goto error;
		else
			renderable->useTexture = VK_TRUE;
	}

	if(	!renderableCreateDescriptorSetLayout(renderable) &&
		!renderableCreateDescriptorPool(renderable) &&
		!renderableCreateUniformsBuffer(renderable) &&
		!renderableCreateDescriptorSets(renderable) &&
		!renderableLoadShaders(renderable) &&
		!renderableCreatePipelineLayout(renderable) &&
		!renderableCreatePipeline(renderable))
	{
		sceneAddRenderable(engine.scene, renderable);
		glm_mat4_identity(renderable->modelMatrix);

		return renderable;
	}

	error:
	renderableDestroy(renderable);

	return NULL;
}


void renderableDestroy(renderable renderable)
{
	// TODO clean a shitload of stuff here !!!
	vkDeviceWaitIdle(engine.vulkanContext.device);

	vkDestroyDescriptorPool(engine.vulkanContext.device, renderable->descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(engine.vulkanContext.device, renderable->descriptorSetLayout, NULL);

	vkDestroyPipeline(engine.vulkanContext.device, renderable->pipeline, NULL);
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->fragShader, NULL);
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->vertShader, NULL);
	vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);

	if(renderable->useUniforms)
	{
		for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(engine.vulkanContext.device, renderable->uniformBuffers[i], NULL);
			vkFreeMemory(engine.vulkanContext.device, renderable->uniformBuffersMemory[i], NULL);
		}
	}

	geometryDestroy(renderable->geometry);

	if(renderable->useTexture)
		textureDestroy(renderable->texture);

	free(renderable);
}


int renderableLoadShaders(renderable renderable)
{
	if (loadShader(engine.vulkanContext.device, renderable->vertShaderName, &renderable->vertShader))
		return -1;

	if (loadShader(engine.vulkanContext.device, renderable->fragShaderName, &renderable->fragShader))
		return -1;

	renderable->shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderable->shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	renderable->shaderStages[0].module = renderable->vertShader;
	renderable->shaderStages[0].pName = "main";
	// TODO : check vertShaderStageInfo.pSpecializationInfo to pass constants to the shaders

	renderable->shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderable->shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	renderable->shaderStages[1].module = renderable->fragShader;
	renderable->shaderStages[1].pName = "main";

	return 0;
}


int renderableCreatePipelineLayout(renderable renderable)
{
	// Required here even when we don't use it
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	if(renderable->useUniforms)
	{
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderable->descriptorSetLayout;
	}

	// Push constant
	VkPushConstantRange pushConstantRange =	{
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = sizeof(float), // TODO remove this hardcoded...
		.offset = 0
	};

	if(renderable->usePushConstant)
	{
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	}

	if (vkCreatePipelineLayout(engine.vulkanContext.device, &pipelineLayoutInfo, NULL, &renderable->pipelineLayout))
	{
		logError("Failed to create pipeline layout");

		return -1;
	}

	return 0;
}


int renderableCreatePipeline(renderable renderable)
{
	// vertex attributes
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	geometryFillVertexInputInfo(renderable->geometry, &vertexInputInfo);

	// type of geometry we want to draw
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = renderable->primitiveTopology,
		.primitiveRestartEnable = VK_FALSE
	};

	// even though they are dynamically set, we need to specify their count here
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};

	// rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE, // if enabled, clamp depth instead of discarding fragments
		.rasterizerDiscardEnable = VK_FALSE, // https://stackoverflow.com/questions/42470669/when-does-it-make-sense-to-turn-off-the-rasterization-step
		.polygonMode = renderable->polygonMode,
		.lineWidth = 1,
		// culling
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE // see rasterizer.depth* for moar depth control
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = engine.vulkanContext.maxMSAA,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 0.2f, // Optional
		.pSampleMask = NULL, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE // Optional
	};

	// depth and stencil buffer
	//VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO}

	// Basic color blending for one framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD
	};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		// if true, almost all of colorBlendAttachment is ignored, and colorBlending.logicOp specifies the blend operation
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1, // Only one framebuffer
		.pAttachments = &colorBlendAttachment,
		// .blendConstants to fix the constants of some blend operations (e.g. VK_BLEND_FACTOR_CONSTANT_COLOR)
	};

	// Dynamic states are specified in the command buffer instead of the pipeline
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = sizeof dynamicStates / sizeof dynamicStates[0],
		.pDynamicStates = dynamicStates
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.stencilTestEnable = VK_FALSE,
		.depthTestEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthWriteEnable = VK_TRUE,
		// allows to only emit fragments on a specific range
		.depthBoundsTestEnable = VK_FALSE,
		//.minDepthBounds = 0;
		//.maxDepthBounds = 1;
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = sizeof renderable->shaderStages / sizeof renderable->shaderStages[0],
		.pStages = renderable->shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState = NULL,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = renderable->pipelineLayout,
		.renderPass = engine.vulkanContext.renderPass,
		.subpass = 0, // index of our only subpass
		// TODO those two are needed only when re-using another existing pipeline
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if(engine.vulkanContext.useDepthBuffer)
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
	else
		pipelineInfo.pDepthStencilState = NULL;

	// TODO interesting second parameter : pipeline cache can be used to speedup all this, even across runs, through a file !
	if (vkCreateGraphicsPipelines(engine.vulkanContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &renderable->pipeline))
	{
		logError("Failed to create graphic pipeline");

		return -1;
	}

	return 0;
}


void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline);

	// bind descriptor set to send uniforms
	if(renderable->useUniforms)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineLayout, 0, 1, &renderable->descriptorSets[engine.vulkanContext.currentFrame], 0, NULL);

	// push constant
	if(renderable->usePushConstant)
		vkCmdPushConstants(commandBuffer, renderable->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &renderable->pushConstantAlpha);

	geometryDraw(renderable->geometry, commandBuffer);
}


int renderableCreateUniformsBuffer(renderable renderable)
{
	if(renderable->useUniforms)
	{
		for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			bufferCreate(renderable->uniformSize, &renderable->uniformBuffers[i], &renderable->uniformBuffersMemory[i],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}

	return 0;
}


int renderableUpdateUniformsBuffer(renderable renderable, const void *data)
{
	if(renderable->useUniforms == VK_FALSE)
		return -1;

	void *dest;
	VkDeviceMemory deviceMemory = renderable->uniformBuffersMemory[engine.vulkanContext.currentFrame];

	vkMapMemory(engine.vulkanContext.device, deviceMemory, 0, renderable->uniformSize, 0, &dest);
	memcpy(dest, data, renderable->uniformSize);
	vkUnmapMemory(engine.vulkanContext.device, deviceMemory);

	return 0;
}


int renderableCreateDescriptorSetLayout(renderable renderable)
{
	VkDescriptorSetLayoutBinding bindings[MAX_BINDINGS];
	uint32_t bindingCount = 0;

	if(renderable->useUniforms)
	{
		VkDescriptorSetLayoutBinding vertexAttributesLayoutBinding = {
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.binding = 0,
			.descriptorCount = 1, // number of element, > 1 if we pass an array
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // * shader stage
			.pImmutableSamplers = NULL // for images
		};

		bindings[bindingCount++ % MAX_BINDINGS] = vertexAttributesLayoutBinding;
	}

	if(renderable->useTexture)
	{
		VkDescriptorSetLayoutBinding textureSamplerLayoutBinding = {
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.binding = 1,
			.descriptorCount = 1, // number of element, > 1 if we pass an array
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // * shader stage
			.pImmutableSamplers = NULL
		};

		bindings[bindingCount++ % MAX_BINDINGS] = textureSamplerLayoutBinding;
	}

	if(bindingCount > MAX_BINDINGS)
	{
		logError("Reached MAX_BINDINGS");

		return -1;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = bindingCount,
		.pBindings = bindings
	};

	if(vkCreateDescriptorSetLayout(engine.vulkanContext.device, &layoutInfo, NULL, &renderable->descriptorSetLayout))
		return -1;

	return 0;
}


int renderableCreateDescriptorPool(renderable renderable)
{
	VkDescriptorPoolSize poolSizes[MAX_BINDINGS];
	uint32_t poolSizeCount = 0;

	if(renderable->useUniforms)
	{
		poolSizes[poolSizeCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[poolSizeCount].descriptorCount = MAX_FRAMES_IN_FLIGHT;
		poolSizeCount++;
	}

	if(renderable->useTexture)
	{
		poolSizes[poolSizeCount].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[poolSizeCount].descriptorCount = MAX_FRAMES_IN_FLIGHT;
		poolSizeCount++;
	}

	if(poolSizeCount > MAX_BINDINGS)
	{
		logError("Reached MAX_BINDINGS");

		return -1;
	}

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = poolSizeCount,
		.pPoolSizes = poolSizes,
		.maxSets = MAX_FRAMES_IN_FLIGHT // maximum number of descriptor sets that can be allocated from the pool
	};

	if(vkCreateDescriptorPool(engine.vulkanContext.device, &descriptorPoolInfo, NULL, &renderable->descriptorPool))
	{
		logError("vkCreateDescriptorPool() failed");

		return -1;
	}

	return 0;
}


int renderableCreateDescriptorSets(renderable renderable)
{
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		layouts[i] = renderable->descriptorSetLayout;

	VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = renderable->descriptorPool,
		.pSetLayouts = layouts,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT
	};

	VkResult result = vkAllocateDescriptorSets(engine.vulkanContext.device, &allocInfo, renderable->descriptorSets);

	if(result)
	{
		logError("vkAllocateDescriptorSets() failed)");

		return -1;
	}

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkWriteDescriptorSet descriptorWrites[MAX_BINDINGS];
		memset(descriptorWrites, 0, sizeof descriptorWrites);
		uint32_t descriptorWriteCount = 0;

		VkDescriptorBufferInfo bufferInfo;

		if(renderable->useUniforms)
		{
			bufferInfo.buffer = renderable->uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = renderable->uniformSize; // could use VK_WHOLE_SIZE here

			descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[descriptorWriteCount].dstSet = renderable->descriptorSets[i];
			descriptorWrites[descriptorWriteCount].dstBinding = 0;	// * here is the binding that matches the glsl code
			descriptorWrites[descriptorWriteCount].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
			descriptorWrites[descriptorWriteCount].descriptorCount = 1; // * only one element to transfer
			descriptorWrites[descriptorWriteCount].pBufferInfo = &bufferInfo;
			descriptorWriteCount++;
		}

		VkDescriptorImageInfo imageInfo;

		if(renderable->useTexture)
		{
			imageInfo.sampler = engine.vulkanContext.textureSampler;
			imageInfo.imageView = renderable->texture->imageView;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[descriptorWriteCount].dstSet = renderable->descriptorSets[i];
			descriptorWrites[descriptorWriteCount].dstBinding = 1;	// * here is the binding that matches the glsl code
			descriptorWrites[descriptorWriteCount].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
			descriptorWrites[descriptorWriteCount].descriptorCount = 1; // * only one element to transfer
			descriptorWrites[descriptorWriteCount].pImageInfo = &imageInfo;
			descriptorWriteCount++;
		}

		vkUpdateDescriptorSets(engine.vulkanContext.device, descriptorWriteCount, descriptorWrites, 0, NULL);
	}

	return result;
}


int renderableUpdatePushConstant(renderable renderable, float alpha)
{
	if(renderable->usePushConstant == VK_FALSE)
		return -1;

	renderable->pushConstantAlpha = alpha;

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		engine.vulkanContext.needCommandBufferUpdate[i] = VK_TRUE;

	return 0;
}


void renderableSetMatrix(renderable renderable, mat4 matrix)
{
	// TODO fix this awful hack
	glm_mat4_copy(matrix, renderable->modelMatrix);

	modelViewProj mvp;

	glm_mat4_copy(renderable->modelMatrix, mvp.model);
	cameraGetView(sceneGetCamera(engine.scene), mvp.view);
	cameraGetProj(sceneGetCamera(engine.scene), mvp.projection);

	renderableUpdateUniformsBuffer(renderable, &mvp);
}
