#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "buffer.h"
#include "core.h"
#include "texture.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName)
{
	renderable renderable = calloc(1, sizeof(*renderable));

	renderable->vertShaderName = vertShaderName;
	renderable->fragShaderName = fragShaderName;
	renderable->geometry = geometry;
	renderable->isReady = VK_FALSE;
	renderable->useTexture = VK_TRUE;

	return renderable;
}


int makeReady(renderable renderable)
{
	if(renderable->isReady)
		return 0;

	if(!textureCreate("holes.png", &renderable->textureImage, &renderable->textureImageMemory) &&
		!createImageView2D(renderable->textureImage, VK_FORMAT_R8G8B8A8_SRGB, &renderable->textureImageView) &&
		!createDescriptorSetLayout(renderable) &&
		!createDescriptorPool(renderable) &&
		!createUniformsBuffer(renderable) &&
		!createDescriptorSets(renderable) &&
		!loadShaders(renderable) &&
		!createPipelineLayout(renderable) &&
		!createPipeline(renderable))
	{
		renderable->isReady = VK_TRUE;

		return 0;
	}

	return -1;
}


void denymDestroyRenderable(renderable renderable)
{
	// TODO clean a shitload of stuff here !!!
	vkDeviceWaitIdle(engine.vulkanContext.device);

	vkDestroyDescriptorPool(engine.vulkanContext.device, renderable->descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(engine.vulkanContext.device, renderable->descriptorSetLayout, NULL);

	vkDestroyPipeline(engine.vulkanContext.device, renderable->pipeline, NULL);
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->fragShader, NULL);
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->vertShader, NULL);
	vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(engine.vulkanContext.device, renderable->uniformBuffers[i], NULL);
		vkFreeMemory(engine.vulkanContext.device, renderable->uniformBuffersMemory[i], NULL);
	}

	geometryDestroy(renderable->geometry);

	// texture
	vkDestroyImageView(engine.vulkanContext.device, renderable->textureImageView, NULL);
	vkDestroyImage(engine.vulkanContext.device, renderable->textureImage, NULL);
	vkFreeMemory(engine.vulkanContext.device, renderable->textureImageMemory, NULL);

	free(renderable);
}


int loadShaders(renderable renderable)
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


int createPipelineLayout(renderable renderable)
{
	// Required here even when we don't use it
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	if(renderable->useUniforms)
	{
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderable->descriptorSetLayout;
	}

	// Push constant
	VkPushConstantRange pushConstantRange =
	{
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = sizeof(float),
		.offset = 0
	};

	if(renderable->usePushConstant)
	{
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	}

	if (vkCreatePipelineLayout(engine.vulkanContext.device, &pipelineLayoutInfo, NULL, &renderable->pipelineLayout))
	{
		fprintf(stderr, "Failed to create pipeline layout");

		return -1;
	}

	return 0;
}


int createPipeline(renderable renderable)
{
	// vertex attributes
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.vertexBindingDescriptionCount = renderable->geometry->attribCount;
	vertexInputInfo.vertexAttributeDescriptionCount = renderable->geometry->attribCount;
	vertexInputInfo.pVertexAttributeDescriptions = renderable->geometry->vertexAttributeDescriptions;
	vertexInputInfo.pVertexBindingDescriptions = renderable->geometry->vertexBindingDescriptions;

	// type of geometry we want to draw
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewport taking all the space available
	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)engine.vulkanContext.swapchainExtent.width;
	viewport.height = (float)engine.vulkanContext.swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// no scissoring
	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = engine.vulkanContext.swapchainExtent;

	// aggregate viewport and scissor
	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE; // if enabled, clamp depth instead of discarding fragments
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // https://stackoverflow.com/questions/42470669/when-does-it-make-sense-to-turn-off-the-rasterization-step
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // lines/points/rectangles
	rasterizer.lineWidth = 1; // needed
	// culling
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE; // see rasterizer.depth* for moar depth control

	// multisampling disabled
	VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = NULL; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// depth and stencil buffer
	//VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO}

	// Basic color blending for one framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	// if true, almost all of colorBlendAttachment is ignored, and colorBlending.logicOp specifies the blend operation
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1; // Only one framebuffer
	colorBlending.pAttachments = &colorBlendAttachment;
	// colorBlending.blendConstants to fix the constants of some blend operations (e.g. VK_BLEND_FACTOR_CONSTANT_COLOR)

	/* If we list some of the previous stages here, they'll become dynamic and we'll
	be able to change their value throughout the life of the pipeline
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof dynamicStates / sizeof dynamicStates[0];
	dynamicState.pDynamicStates = dynamicStates;
	*/

	VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = sizeof renderable->shaderStages / sizeof renderable->shaderStages[0];
	pipelineInfo.pStages = renderable->shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = NULL;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL; // TODO &dynamicState;
	pipelineInfo.layout = renderable->pipelineLayout;
	pipelineInfo.renderPass = engine.vulkanContext.renderPass;
	pipelineInfo.subpass = 0; // index of our only subpass
	// TODO those two are needed only when re-using another existing pipeline
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE ;
	pipelineInfo.basePipelineIndex = -1;

	// TODO interesting second parameter : pipeline cache can be used to speedup all this, even across runs, through a file !
	if (vkCreateGraphicsPipelines(engine.vulkanContext.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &renderable->pipeline))
	{
		fprintf(stderr, "Failed to create graphic pipeline.\n");

		return -1;
	}

	return 0;
}


int recreatePipeline(renderable renderable)
{
	vkDestroyPipeline(engine.vulkanContext.device, renderable->pipeline, NULL);

	return createPipeline(renderable);
}


void renderableDraw(renderable renderable, VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline);

	// bind vertex attributes, except indices
	if(renderable->geometry->attribCount)
	{
		VkBuffer buffers[renderable->geometry->attribCount];
		VkDeviceSize offsets[renderable->geometry->attribCount];

		int index = 0;

		if(renderable->geometry->bufferPositions)
		{
			buffers[index] = renderable->geometry->bufferPositions;
			offsets[index] = 0;
			index++;
		}

		if(renderable->geometry->bufferColors)
		{
			buffers[index] = renderable->geometry->bufferColors;
			offsets[index] = 0;
			index++;
		}

		if(renderable->geometry->bufferTexCoords)
		{
			buffers[index] = renderable->geometry->bufferTexCoords;
			offsets[index] = 0;
			index++;
		}

		if(renderable->geometry->indices)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, renderable->geometry->attribCount - 1, buffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, renderable->geometry->bufferIndices, 0, VK_INDEX_TYPE_UINT16);
		}
		else
			vkCmdBindVertexBuffers(commandBuffer, 0, renderable->geometry->attribCount, buffers, offsets);
	}

	// bind descriptor set to send uniforms
	if(renderable->useUniforms)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineLayout, 0, 1, &renderable->descriptorSets[engine.vulkanContext.currentFrame], 0, NULL);

	// push constant
	if(renderable->usePushConstant)
		vkCmdPushConstants(commandBuffer, renderable->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &renderable->pushConstantAlpha);

	if(renderable->geometry->indices)
		vkCmdDrawIndexed(commandBuffer, renderable->geometry->indexCount, 1, 0, 0, 0);
	else
		vkCmdDraw(commandBuffer, renderable->geometry->vertexCount, 1, 0, 0);
}


int useUniforms(renderable renderable)
{
	if(renderable->isReady)
		return -1;

	renderable->useUniforms = VK_TRUE;

	return 0;
}


int createUniformsBuffer(renderable renderable)
{
	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		createBuffer(sizeof(modelViewProj), &renderable->uniformBuffers[i], &renderable->uniformBuffersMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	return 0;
}


int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp)
{
	if(renderable->isReady == VK_FALSE || renderable->useUniforms == VK_FALSE)
		return -1;

	void *dest;
	VkDeviceMemory deviceMemory = renderable->uniformBuffersMemory[engine.vulkanContext.currentFrame];
	VkDeviceSize size = sizeof * mvp;

	vkMapMemory(engine.vulkanContext.device, deviceMemory, 0, size, 0, &dest);
	memcpy(dest, mvp, size);
	vkUnmapMemory(engine.vulkanContext.device, deviceMemory);

	return 0;
}


int createDescriptorSetLayout(renderable renderable)
{
	VkDescriptorSetLayoutBinding vertexAttributesLayoutBinding;
	vertexAttributesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertexAttributesLayoutBinding.binding = 0;
	vertexAttributesLayoutBinding.descriptorCount = 1; // number of element, > 1 if we pass an array
	vertexAttributesLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // * shader stage
	vertexAttributesLayoutBinding.pImmutableSamplers = NULL; // for images

	VkDescriptorSetLayoutBinding textureSamplerLayoutBinding;
	textureSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureSamplerLayoutBinding.binding = 1;
	textureSamplerLayoutBinding.descriptorCount = 1; // number of element, > 1 if we pass an array
	textureSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // * shader stage
	textureSamplerLayoutBinding.pImmutableSamplers = NULL;

	// TODO: this has to be dynamically filled
	VkDescriptorSetLayoutBinding bindings[2] = { vertexAttributesLayoutBinding, textureSamplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = sizeof bindings / sizeof *bindings;
	layoutInfo.pBindings = bindings;

	if(vkCreateDescriptorSetLayout(engine.vulkanContext.device, &layoutInfo, NULL, &renderable->descriptorSetLayout))
		return -1;

	return 0;
}


int createDescriptorPool(renderable renderable)
{
	// TODO: this has to be dynamically filled
	VkDescriptorPoolSize poolSize[2];
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolInfo.poolSizeCount = sizeof poolSize / sizeof *poolSize;
	descriptorPoolInfo.pPoolSizes = poolSize;
	descriptorPoolInfo.maxSets = MAX_FRAMES_IN_FLIGHT; // maximum number of descriptor sets that can be allocated from the pool

	if(vkCreateDescriptorPool(engine.vulkanContext.device, &descriptorPoolInfo, NULL, &renderable->descriptorPool))
	{
		fprintf(stderr, "vkCreateDescriptorPool() failed)\n");

		return -1;
	}

	return 0;
}


int createDescriptorSets(renderable renderable)
{
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		layouts[i] = renderable->descriptorSetLayout;

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = renderable->descriptorPool;
	allocInfo.pSetLayouts = layouts;
	allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;

	VkResult result = vkAllocateDescriptorSets(engine.vulkanContext.device, &allocInfo, renderable->descriptorSets);

	if(result)
	{
		fprintf(stderr, "vkAllocateDescriptorSets() failed)\n");

		return -1;
	}

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		// TODO: this has to be dynamically filled
		VkWriteDescriptorSet writeDescriptorSets[2] = {0};

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = renderable->uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(modelViewProj); // could use VK_WHOLE_SIZE here

		memset(&writeDescriptorSets[0], 0, sizeof writeDescriptorSets[0]);
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].dstSet = renderable->descriptorSets[i];
		writeDescriptorSets[0].dstBinding = 0;	// * here is the binding that matches the glsl code
		writeDescriptorSets[0].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
		writeDescriptorSets[0].descriptorCount = 1; // * only one element to transfer
		writeDescriptorSets[0].pBufferInfo = &bufferInfo;

		VkDescriptorImageInfo imageInfo;
		imageInfo.sampler = engine.vulkanContext.textureSampler;
		imageInfo.imageView = renderable->textureImageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		memset(&writeDescriptorSets[1], 0, sizeof writeDescriptorSets[1]);
		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].dstSet = renderable->descriptorSets[i];
		writeDescriptorSets[1].dstBinding = 1;	// * here is the binding that matches the glsl code
		writeDescriptorSets[1].dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
		writeDescriptorSets[1].descriptorCount = 1; // * only one element to transfer
		writeDescriptorSets[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(engine.vulkanContext.device, sizeof writeDescriptorSets / sizeof *writeDescriptorSets, writeDescriptorSets, 0, NULL);
	}

	return result;
}


int usePushConstants(renderable renderable)
{
	if(renderable->isReady)
		return -1;

	renderable->usePushConstant = VK_TRUE;

	return 0;
}

int updatePushConstants(renderable renderable, float alpha)
{
	if(renderable->isReady == VK_FALSE || renderable->usePushConstant == VK_FALSE)
		return -1;

	renderable->pushConstantAlpha = alpha;
	engine.vulkanContext.needCommandBufferUpdate = VK_TRUE;

	return 0;
}
