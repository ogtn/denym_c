#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "buffer.h"
#include "core.h"

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

	return renderable;
}


int makeReady(renderable renderable)
{
	if(renderable->isReady)
		return 0;

	if(!createDescriptorSetLayout(renderable) &&
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
	vkDestroyDescriptorPool(engine.vulkanContext.device, renderable->uniformDescriptorPool, NULL);
	vkDestroyDescriptorSetLayout(engine.vulkanContext.device, renderable->uniformDescriptorSetLayout, NULL);

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
		pipelineLayoutInfo.pSetLayouts = &renderable->uniformDescriptorSetLayout;
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
	geometryFillPipelineVertexInputStateCreateInfo(renderable->geometry, &vertexInputInfo);

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

	// bind vertex attributes
	if(renderable->geometry->attribCount > 0)
	{
		VkBuffer buffers[] = {
			renderable->geometry->bufferPositions,
			renderable->geometry->bufferColors
		};
		VkDeviceSize offsets[] = {
			0,
			0
		};

		vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);

		if(renderable->geometry->indices)
			vkCmdBindIndexBuffer(commandBuffer, renderable->geometry->bufferIndices, 0, VK_INDEX_TYPE_UINT16);
	}

	// bind descriptor set to send uniforms
	if(renderable->useUniforms)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineLayout, 0, 1, &renderable->uniformDescriptorSets[engine.vulkanContext.currentFrame], 0, NULL);

	// push constant
	if(renderable->usePushConstant)
		vkCmdPushConstants(commandBuffer, renderable->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &renderable->pushConstantAlpha);

	if(renderable->geometry->indices)
		vkCmdDrawIndexed(commandBuffer, renderable->geometry->vertexCount, 1, 0, 0, 0);
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
	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.binding = 0;
	layoutBinding.descriptorCount = 1; // number of element, > 1 if we pass an array
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // * shader stage
	layoutBinding.pImmutableSamplers = NULL; // for images

	VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if(vkCreateDescriptorSetLayout(engine.vulkanContext.device, &layoutInfo, NULL, &renderable->uniformDescriptorSetLayout))
		return -1;

	return 0;
}


int createDescriptorPool(renderable renderable)
{
	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = &poolSize;
	descriptorPoolInfo.maxSets = MAX_FRAMES_IN_FLIGHT; // maximum number of descriptor sets that can be allocated from the pool 

	if(vkCreateDescriptorPool(engine.vulkanContext.device, &descriptorPoolInfo, NULL, &renderable->uniformDescriptorPool))
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
		layouts[i] = renderable->uniformDescriptorSetLayout;

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = renderable->uniformDescriptorPool;
	allocInfo.pSetLayouts = layouts;
	allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;

	VkResult result = vkAllocateDescriptorSets(engine.vulkanContext.device, &allocInfo, renderable->uniformDescriptorSets);

	if(result)
	{
		fprintf(stderr, "vkAllocateDescriptorSets() failed)\n");

		return -1;
	}

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = renderable->uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(modelViewProj); // could use VK_WHOLE_SIZE here

		VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstSet = renderable->uniformDescriptorSets[i];
		writeDescriptorSet.dstBinding = 0;	// * here is the binding that matches the glsl code
		writeDescriptorSet.dstArrayElement = 0; // * this is an offset, here 0 because we're not sending an array
		writeDescriptorSet.descriptorCount = 1; // * only one element to transfer

		// three choices here :
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		// TODO let's save those ones for later
		writeDescriptorSet.pImageInfo = NULL;
		writeDescriptorSet.pTexelBufferView = NULL;

		vkUpdateDescriptorSets(engine.vulkanContext.device, 1, &writeDescriptorSet, 0, NULL);
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
