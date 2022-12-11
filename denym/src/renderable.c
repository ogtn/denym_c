#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "buffer.h"
#include "core.h"
#include "texture.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


renderable renderableCreate(const renderableCreateParams *params, uint32_t instanceCount)
{
	renderable renderable = calloc(1, sizeof(*renderable));

	renderable->geometry = params->geometry;
	renderable->compactMVP = params->compactMVP;
	renderable->instanceCount = instanceCount;

	glm_mat4_identity(renderable->modelMatrix);

	if(params->sendMVPAsPushConstant || params->sendMVPAsStorageBuffer || params->sendMVP)
	{
		renderable->sendMVP = VK_TRUE;

		for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			renderable->needMVPUpdate[i] = VK_TRUE;
	}

	if(params->sendMVPAsPushConstant)
	{
		if(renderable->instanceCount > 1)
		{
			logError("sendMVPAsPushConstant is not supported with multiple instances");
			goto error;
		}

		renderable->sendMVPAsPushConstant = VK_TRUE;
		renderable->compactMVP = VK_TRUE;
		renderableAddPushConstant(renderable, sizeof(mat4), VK_SHADER_STAGE_VERTEX_BIT);
	}
	else if(params->sendMVPAsStorageBuffer)
	{
		renderable->useStorageBuffer = VK_TRUE;
		renderable->sendMVPAsStorageBuffer = VK_TRUE;

		if(renderable->compactMVP)
			renderable->storageBufferSizePerFrameAndInstance = sizeof(mat4); // only one mvp matrix per instance
		else
			renderable->storageBufferSizePerFrameAndInstance = sizeof(mat4) * 3; // model, view, proj for each instance

		renderable->storageBufferSizePerFrame = renderable->storageBufferSizePerFrameAndInstance * renderable->instanceCount;
		renderable->storageBufferTotalSize = renderable->storageBufferSizePerFrame * MAX_FRAMES_IN_FLIGHT;
	}
	else if(params->sendMVP)
	{
		if(renderable->instanceCount > 1)
		{
			logError("sending MVP as uniforms is not supported with multiple instances");
			goto error;
		}

		renderable->sendMVPAsUniform = VK_TRUE;
		renderable->uniforms.mvpId = renderableAddUniformInternal(renderable, renderable->compactMVP ? sizeof(mat4) : sizeof(mat4) * 3);
	}

	if(params->pushConstantSize)
		renderableAddPushConstant(renderable, params->pushConstantSize, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);

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

	if(params->sendLigths)
	{
		renderable->uniforms.dlightId = renderableAddUniformInternal(renderable, sizeof(dlight_t));
		renderable->uniforms.plightId = renderableAddUniformInternal(renderable, sizeof engine.scene->plights);
		renderable->sendLights = VK_TRUE;
	}

	if(params->material || params->sendLigths)
	{
		renderable->uniforms.materialId = renderableAddUniformInternal(renderable, sizeof(material_t));
		renderable->sendMaterial = VK_TRUE;
	}

	if(params->material)
		renderable->material = *params->material;
	else
	{
		// default material if none provided
		renderable->material.color.r = 1;
		renderable->material.color.g = 1;
		renderable->material.color.b = 1;
		renderable->material.shininess = 100;
	}

	if(	!renderableCreateDescriptorSetLayout(renderable) &&
		!renderableCreateDescriptorPool(renderable) &&
		!renderableCreateUniformsBuffers(renderable) &&
		!renderableCreateStorageBuffer(renderable) &&
		!renderableCreateDescriptorSets(renderable, params->useNearestSampler) &&
		!renderableLoadShaders(renderable, params->vertShaderName, params->fragShaderName) &&
		!renderableCreatePipelineLayout(renderable) &&
		!renderableCreatePipeline(renderable))
	{
		sceneAddRenderable(engine.scene, renderable);

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
	shaderDestroy(renderable->vertShader);
	shaderDestroy(renderable->fragShader);
	vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);

	for(uint32_t id = 0; id < renderable->uniforms.count; id++)
	{
		if(engine.settings.cacheUniformMemory)
		{
			vkUnmapMemory(engine.vulkanContext.device, renderable->uniforms.buffersMemory[id]);
			renderable->uniforms.cache[id] = NULL;
		}

		vkDestroyBuffer(engine.vulkanContext.device, renderable->uniforms.buffers[id], NULL);
		vkFreeMemory(engine.vulkanContext.device, renderable->uniforms.buffersMemory[id], NULL);
	}

	if(renderable->useStorageBuffer)
	{
		if(engine.settings.cacheStorageBufferMemory)
		{
			vkUnmapMemory(engine.vulkanContext.device, renderable->storageBufferMemory);
			renderable->storageBufferCache = NULL;
		}

		vkDestroyBuffer(engine.vulkanContext.device, renderable->storageBuffer, NULL);
		vkFreeMemory(engine.vulkanContext.device, renderable->storageBufferMemory, NULL);
	}

	for(uint32_t i = 0; i < renderable->pushConstants.count; i++)
		free(renderable->pushConstants.values[i]);

	geometryDestroy(renderable->geometry);

	if(renderable->useTexture)
		textureDestroy(renderable->texture);

	free(renderable);
}


int renderableLoadShaders(renderable renderable, const char *vertShaderName, const char *fragShaderName)
{
	if((renderable->vertShader = shaderCreate(engine.vulkanContext.device, vertShaderName)) ==  NULL)
		return -1;

	if((renderable->fragShader = shaderCreate(engine.vulkanContext.device, fragShaderName)) == NULL)
		return -1;

	renderable->shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderable->shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	renderable->shaderStages[0].module = renderable->vertShader->shaderModule;
	renderable->shaderStages[0].pName = "main";
	// TODO : check vertShaderStageInfo.pSpecializationInfo to pass constants to the shaders

	renderable->shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	renderable->shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	renderable->shaderStages[1].module = renderable->fragShader->shaderModule;
	renderable->shaderStages[1].pName = "main";

	return 0;
}


int renderableCreatePipelineLayout(renderable renderable)
{
	// Required here even when we don't use it
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	if(renderable->uniforms.count || renderable->useStorageBuffer || renderable->useTexture)
	{
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderable->descriptorSetLayout;
	}

	// Push constant
	VkPushConstantRange pushConstantRanges[RENDERABLE_MAX_PUSH_CONSTANTS];

	if(renderable->pushConstants.count)
	{
		uint32_t offset = 0;

		for(uint32_t i = 0; i <  renderable->pushConstants.count; i++)
		{
			pushConstantRanges[i].offset = offset;
			pushConstantRanges[i].size = renderable->pushConstants.sizes[i];
			pushConstantRanges[i].stageFlags = renderable->pushConstants.shaderStages[i];
			offset += pushConstantRanges[i].size;
		}

		pipelineLayoutInfo.pushConstantRangeCount = renderable->pushConstants.count;
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
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
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE // see rasterizer.depth* for moar depth control
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = engine.vulkanContext.MSAASampling,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 0.2f, // Optional
		.pSampleMask = NULL, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE // Optional
	};

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

	if(engine.settings.useDepthBuffer)
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
	else
		pipelineInfo.pDepthStencilState = NULL;

	if (vkCreateGraphicsPipelines(engine.vulkanContext.device, engine.caches.pipelineCache, 1, &pipelineInfo, NULL, &renderable->pipeline))
	{
		logError("Failed to create graphic pipeline");

		return -1;
	}

	return 0;
}


void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline);

	// bind descriptor set to send uniforms, storage buffer and texture sampler
	if(renderable->uniforms.count || renderable->useStorageBuffer || renderable->useTexture)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineLayout, 0, 1, &renderable->descriptorSets[engine.vulkanContext.currentFrame], 0, NULL);

	// push constant
	for(uint32_t i = 0; i < renderable->pushConstants.count; i++)
		vkCmdPushConstants(commandBuffer, renderable->pipelineLayout, renderable->pushConstants.shaderStages[i], 0, renderable->pushConstants.sizes[i], renderable->pushConstants.values[i]);

	geometryDraw(renderable->geometry, commandBuffer, renderable->instanceCount);
}


uint32_t renderableAddUniformInternal(renderable renderable, VkDeviceSize size)
{
	uint32_t id = renderable->uniforms.count;

	if(id >= RENDERABLE_MAX_UNIFORMS)
	{
		logError("Reached RENDERABLE_MAX_UNIFORMS");

		return UINT32_MAX;
	}

	VkDeviceSize align = engine.vulkanContext.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

	if(size % align)
		renderable->uniforms.sizePerFrame[id] = (size / align + 1) * align;
	else
		renderable->uniforms.sizePerFrame[id] = size;

	renderable->uniforms.count++;

	return id;
}


int renderableCreateUniformsBuffers(renderable renderable)
{
	for(uint32_t id = 0; id < renderable->uniforms.count; id++)
	{
		bufferCreate(
			renderable->uniforms.sizePerFrame[id] * MAX_FRAMES_IN_FLIGHT,
			&renderable->uniforms.buffers[id],
			&renderable->uniforms.buffersMemory[id],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		if(engine.settings.cacheUniformMemory)
			vkMapMemory(
				engine.vulkanContext.device, renderable->uniforms.buffersMemory[id], 0,
				renderable->uniforms.sizePerFrame[id] * MAX_FRAMES_IN_FLIGHT, 0, &renderable->uniforms.cache[id]);
	}

	return 0;
}


int renderableCreateStorageBuffer(renderable renderable)
{
	if(renderable->useStorageBuffer)
	{
		bufferCreate(
			renderable->storageBufferTotalSize,
			&renderable->storageBuffer,
			&renderable->storageBufferMemory,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		if(engine.settings.cacheStorageBufferMemory)
			vkMapMemory(engine.vulkanContext.device, renderable->storageBufferMemory, 0, renderable->storageBufferTotalSize, 0, &renderable->storageBufferCache);
	}

	return 0;
}


int renderableUpdateUniformsBuffer(renderable renderable, uint32_t id, const void *data)
{
	if(id >= renderable->uniforms.count)
	{
		logError("Can't update uniform %u, renderable has only %u", id, renderable->uniforms.count);

		return -1;
	}

	uint64_t offset = renderable->uniforms.sizePerFrame[id] * engine.vulkanContext.currentFrame;

	if(engine.settings.cacheUniformMemory)
		memcpy(renderable->uniforms.cache[id] + offset, data, renderable->uniforms.sizePerFrame[id]);
	else
	{
		vkMapMemory(
			engine.vulkanContext.device, renderable->uniforms.buffersMemory[id], offset,
			renderable->uniforms.sizePerFrame[id], 0, &renderable->uniforms.cache[id]);
		memcpy(renderable->uniforms.cache[id], data, renderable->uniforms.sizePerFrame[id]);
		vkUnmapMemory(engine.vulkanContext.device, renderable->uniforms.buffersMemory[id]);
		renderable->uniforms.cache[id] = NULL;
	}

	return 0;
}


int renderableUpdateStorageBuffer(renderable renderable, const void *data, uint32_t instanceId)
{
	if(renderable->useStorageBuffer == VK_FALSE)
		return -1;

	uint64_t offset = renderable->storageBufferSizePerFrame * engine.vulkanContext.currentFrame
		+ renderable->storageBufferSizePerFrameAndInstance * instanceId;

	if(engine.settings.cacheStorageBufferMemory)
		memcpy(renderable->storageBufferCache + offset, data, renderable->storageBufferSizePerFrameAndInstance);
	else
	{
		vkMapMemory(engine.vulkanContext.device, renderable->storageBufferMemory, offset, renderable->storageBufferSizePerFrameAndInstance, 0, &renderable->storageBufferCache);
		memcpy(renderable->storageBufferCache, data, renderable->storageBufferSizePerFrameAndInstance);
		vkUnmapMemory(engine.vulkanContext.device, renderable->storageBufferMemory);
		renderable->storageBufferCache = NULL;
	}

	return 0;
}


int renderableCreateDescriptorSetLayout(renderable renderable)
{
	VkDescriptorSetLayoutBinding bindings[RENDERABLE_MAX_BINDINGS];
	uint32_t bindingCount = 0;

	for(uint32_t id = 0; id < renderable->uniforms.count; id++)
	{
		VkDescriptorSetLayoutBinding vertexAttributesLayoutBinding = {
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.binding = bindingCount,
			.descriptorCount = 1, // number of element, > 1 if we pass an array
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // * shader stage
			.pImmutableSamplers = NULL // for images
		};

		bindings[bindingCount++ % RENDERABLE_MAX_BINDINGS] = vertexAttributesLayoutBinding;
	}

	if(renderable->useStorageBuffer)
	{
		VkDescriptorSetLayoutBinding storageBufferLayoutBinding = {
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.binding = bindingCount,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
		};

		bindings[bindingCount++ % RENDERABLE_MAX_BINDINGS] = storageBufferLayoutBinding;
	}

	if(renderable->useTexture)
	{
		VkDescriptorSetLayoutBinding textureSamplerLayoutBinding = {
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.binding = bindingCount,
			.descriptorCount = 1, // number of element, > 1 if we pass an array
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // * shader stage
			.pImmutableSamplers = NULL
		};

		bindings[bindingCount++ % RENDERABLE_MAX_BINDINGS] = textureSamplerLayoutBinding;
	}

	if(bindingCount > RENDERABLE_MAX_BINDINGS)
	{
		logError("Reached RENDERABLE_MAX_BINDINGS");

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
	VkDescriptorPoolSize poolSizes[RENDERABLE_MAX_BINDINGS];
	uint32_t poolSizeCount = 0;

	for(uint32_t id = 0; id < renderable->uniforms.count; id++)
	{
		poolSizes[poolSizeCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[poolSizeCount].descriptorCount = MAX_FRAMES_IN_FLIGHT;
		poolSizeCount++;
	}

	if(renderable->useStorageBuffer)
	{
		poolSizes[poolSizeCount].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[poolSizeCount].descriptorCount = MAX_FRAMES_IN_FLIGHT;
		poolSizeCount++;
	}

	if(renderable->useTexture)
	{
		poolSizes[poolSizeCount].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[poolSizeCount].descriptorCount = MAX_FRAMES_IN_FLIGHT;
		poolSizeCount++;
	}

	if(poolSizeCount > RENDERABLE_MAX_BINDINGS)
	{
		logError("Reached RENDERABLE_MAX_BINDINGS");

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


int renderableCreateDescriptorSets(renderable renderable, VkBool32 useNearestSampler)
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
		VkWriteDescriptorSet descriptorWrites[RENDERABLE_MAX_BINDINGS];
		memset(descriptorWrites, 0, sizeof descriptorWrites);
		uint32_t descriptorWriteCount = 0;

		VkDescriptorBufferInfo uniformBufferInfo[RENDERABLE_MAX_UNIFORMS];

		for(uint32_t id = 0; id < renderable->uniforms.count; id++)
		{
			uniformBufferInfo[id].buffer = renderable->uniforms.buffers[id];
			uniformBufferInfo[id].range = renderable->uniforms.sizePerFrame[id];
			uniformBufferInfo[id].offset = renderable->uniforms.sizePerFrame[id] * i;

			descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[descriptorWriteCount].dstSet = renderable->descriptorSets[i];
			descriptorWrites[descriptorWriteCount].dstBinding = descriptorWriteCount;	// * here is the binding that matches the glsl code
			descriptorWrites[descriptorWriteCount].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
			descriptorWrites[descriptorWriteCount].descriptorCount = 1; // * only one element to transfer
			descriptorWrites[descriptorWriteCount].pBufferInfo = &uniformBufferInfo[id];
			descriptorWriteCount++;
		}

		VkDescriptorBufferInfo storageBufferInfo;

		if(renderable->useStorageBuffer)
		{
			storageBufferInfo.buffer = renderable->storageBuffer;
			storageBufferInfo.range = renderable->storageBufferSizePerFrame;
			storageBufferInfo.offset = renderable->storageBufferSizePerFrame * i;

			descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteCount].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[descriptorWriteCount].dstSet = renderable->descriptorSets[i];
			descriptorWrites[descriptorWriteCount].dstBinding = descriptorWriteCount;	// * here is the binding that matches the glsl code
			descriptorWrites[descriptorWriteCount].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
			descriptorWrites[descriptorWriteCount].descriptorCount = 1; // * only one element to transfer
			descriptorWrites[descriptorWriteCount].pBufferInfo = &storageBufferInfo;
			descriptorWriteCount++;
		}

		VkDescriptorImageInfo imageInfo;

		if(renderable->useTexture)
		{
			if(useNearestSampler)
				imageInfo.sampler = engine.vulkanContext.textureSamplers.nearest;
			else
				imageInfo.sampler = engine.vulkanContext.textureSamplers.linear;

			imageInfo.imageView = renderable->texture->imageView;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			descriptorWrites[descriptorWriteCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[descriptorWriteCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[descriptorWriteCount].dstSet = renderable->descriptorSets[i];
			descriptorWrites[descriptorWriteCount].dstBinding = descriptorWriteCount;	// * here is the binding that matches the glsl code
			descriptorWrites[descriptorWriteCount].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
			descriptorWrites[descriptorWriteCount].descriptorCount = 1; // * only one element to transfer
			descriptorWrites[descriptorWriteCount].pImageInfo = &imageInfo;
			descriptorWriteCount++;
		}

		if(descriptorWriteCount > RENDERABLE_MAX_BINDINGS)
		{
			logError("Exceed max number of bindings");

			return -1;
		}

		vkUpdateDescriptorSets(engine.vulkanContext.device, descriptorWriteCount, descriptorWrites, 0, NULL);
	}

	return result;
}


int renderableUpdatePushConstant(renderable renderable, void *value)
{
	// todo make this configurable through user push constant indices ?
	return renderableUpdatePushConstantInternal(renderable, value, renderable->pushConstants.count - 1);
}


int renderableUpdatePushConstantInternal(renderable renderable, void *value, uint32_t pushConstantNumber)
{
	if(renderable->pushConstants.count <= pushConstantNumber)
	{
		logWarning("Can't update push constant on this renderable : none defined");

		return -1;
	}

	memcpy(renderable->pushConstants.values[pushConstantNumber], value, renderable->pushConstants.sizes[pushConstantNumber]);

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		engine.vulkanContext.needCommandBufferUpdate[i] = VK_TRUE;

	return 0;
}


void renderableSetMatrix(renderable renderable, mat4 matrix)
{
	if(renderable->instanceCount > 1)
	{
		logWarning("Renderable is instanciated, can't set matrix without specifying instance Id");

		return;
	}

	glm_mat4_copy(matrix, renderable->modelMatrix);

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		renderable->needMVPUpdate[i] = VK_TRUE;
}


void renderableUpdateMVP(renderable renderable, VkBool32 force)
{
	if(renderable->sendMVP == VK_FALSE)
		return;

	if(renderable->needMVPUpdate[engine.vulkanContext.currentFrame] == VK_FALSE && force == VK_FALSE)
		return;

	mat4 matrices[3];

	glm_mat4_copy(renderable->modelMatrix, matrices[0]);

	camera camera = sceneGetCamera(engine.scene);

	if(camera)
	{
		cameraGetView(camera, matrices[1]);
		cameraGetProj(camera, matrices[2]);
	}
	else
	{
		glm_mat4_identity(matrices[1]);
		glm_mat4_identity(matrices[2]);
	}

	if(renderable->compactMVP)
	{
		mat4 mvp;

		glm_mat4_mul(matrices[2], matrices[1], mvp);
		glm_mat4_mul(mvp, matrices[0], mvp);

		if(renderable->sendMVPAsPushConstant)
			renderableUpdatePushConstantInternal(renderable, mvp, 0);
		else if(renderable->sendMVPAsStorageBuffer)
			renderableUpdateStorageBuffer(renderable, mvp, 0);
		else if(renderable->sendMVPAsUniform)
			renderableUpdateUniformsBuffer(renderable, renderable->uniforms.mvpId, mvp);
	}
	else if(renderable->sendMVPAsStorageBuffer)
		renderableUpdateStorageBuffer(renderable, matrices, 0);
	else if(renderable->sendMVPAsUniform)
		renderableUpdateUniformsBuffer(renderable, renderable->uniforms.mvpId, matrices);

	renderable->needMVPUpdate[engine.vulkanContext.currentFrame] = VK_FALSE;
}


void renderableUpdateLighting(renderable renderable)
{
	if(renderable->sendLights)
	{
		renderableUpdateUniformsBuffer(renderable, renderable->uniforms.dlightId, &engine.scene->dlight);
		renderableUpdateUniformsBuffer(renderable, renderable->uniforms.plightId, engine.scene->plights);
	}

	if(renderable->sendMaterial)
		renderableUpdateUniformsBuffer(renderable, renderable->uniforms.materialId, &renderable->material);
}


void renderableSetMatrixInstance(renderable renderable, mat4 matrix, uint32_t instanceId)
{
	if(instanceId >= renderable->instanceCount)
	{
		logWarning("Renderable has only %u instances, instance %u doesn't exist",
			renderable->instanceCount, instanceId);

		return;
	}

	mat4 matrices[3];

	glm_mat4_copy(matrix, matrices[0]);
	cameraGetView(sceneGetCamera(engine.scene), matrices[1]);
	cameraGetProj(sceneGetCamera(engine.scene), matrices[2]);

	if(renderable->compactMVP)
	{
		mat4 mvp;

		glm_mat4_mul(matrices[2], matrices[1], mvp);
		glm_mat4_mul(mvp, matrices[0], mvp);
		renderableUpdateStorageBuffer(renderable, mvp, instanceId);
	}
	else
		renderableUpdateStorageBuffer(renderable, matrices, instanceId);
}


int renderableAddPushConstant(renderable renderable, uint32_t size, VkShaderStageFlags shaderStage)
{
	if(renderable->pushConstants.count >= RENDERABLE_MAX_PUSH_CONSTANTS)
	{
		logWarning("Max number of push constant reached (%d)", RENDERABLE_MAX_PUSH_CONSTANTS);

		return -1;
	}

	if(renderable->pushConstants.totalSize + size > engine.vulkanContext.physicalDeviceProperties.limits.maxPushConstantsSize)
	{
		logWarning("Push constant size (%u) is above the physical device limit (%u)",
			renderable->pushConstants.totalSize + size, engine.vulkanContext.physicalDeviceProperties.limits.maxPushConstantsSize);

		return -1;
	}

	renderable->pushConstants.totalSize += size;
	renderable->pushConstants.sizes[renderable->pushConstants.count] += size;
	renderable->pushConstants.values[renderable->pushConstants.count] = malloc(size);
	renderable->pushConstants.shaderStages[renderable->pushConstants.count] = shaderStage;
	renderable->pushConstants.count++;

	return 0;
}
