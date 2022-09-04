#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "buffer.h"
#include "core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static renderable_t static_renderable;


renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName)
{
	renderable renderable = &static_renderable;

	renderable->vertShaderName = vertShaderName;
	renderable->fragShaderName = fragShaderName;
	renderable->geometry = geometry;
	renderable->needCommandBufferUpdate = VK_TRUE;

	if(!createDescriptorSetLayout(renderable) &&
		!createDescriptorPool(renderable) &&
		!createUniformsBuffer(renderable) &&
		!createDescriptorSets(renderable) &&
		!createPipeline(renderable) &&
		!createBuffers(geometry) &&
		!createCommandBuffers(renderable))
	{
		return renderable;
	}

	return NULL;
}


void denymDestroyRenderable(renderable renderable)
{
	// TODO clean a shitload of stuff here !!!
	vkDestroyDescriptorPool(engine.vulkanContext.device, renderable->uniformDescriptorPool, NULL);
	vkDestroyDescriptorSetLayout(engine.vulkanContext.device, renderable->uniformDescriptorSetLayout, NULL);
	free(renderable->uniformDescriptorSets);

	vkDestroyPipeline(engine.vulkanContext.device, renderable->pipeline, NULL);
	vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);
	vkFreeCommandBuffers(engine.vulkanContext.device, engine.vulkanContext.commandPool, engine.vulkanContext.imageCount, renderable->commandBuffers);
	free(renderable->commandBuffers);

	for(uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
	{
		vkDestroyBuffer(engine.vulkanContext.device, renderable->uniformBuffers[i], NULL);
		vkFreeMemory(engine.vulkanContext.device, renderable->uniformBuffersMemory[i], NULL);
	}

	free(renderable->uniformBuffers);
	free(renderable->uniformBuffersMemory);

	denymDestroyGeometry(renderable->geometry);
}


int createPipeline(renderable renderable)
{
	int result = -1;

	if (loadShader(engine.vulkanContext.device, renderable->vertShaderName, &renderable->vertShader))
		goto err_vert;

	if (loadShader(engine.vulkanContext.device, renderable->fragShaderName, &renderable->fragShader))
		goto err_frag;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = renderable->vertShader;
	vertShaderStageInfo.pName = "main";
	// TODO : check vertShaderStageInfo.pSpecializationInfo to pass constants to the shaders

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = renderable->fragShader;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	// vertex attributes
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	if(renderable->geometry->attribCount > 0)
	{
		vertexInputInfo.vertexBindingDescriptionCount = renderable->geometry->attribCount;
		vertexInputInfo.vertexAttributeDescriptionCount = renderable->geometry->attribCount;

		VkVertexInputAttributeDescription *vertextAttributeDescriptions = malloc(sizeof * vertextAttributeDescriptions * renderable->geometry->attribCount);
		VkVertexInputBindingDescription *vertexBindingDescriptions = malloc(sizeof * vertexBindingDescriptions * renderable->geometry->attribCount);

		// here we could have had positions and colors in the same array
		// in this case, only one vertexBindingDescriptions, and two vertextAttributeDescriptions

		if(renderable->geometry->positions)
		{
			vertextAttributeDescriptions[0].binding = 0;
			vertextAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
			vertextAttributeDescriptions[0].location = 0;
			vertextAttributeDescriptions[0].offset = 0; // because only one type of data in this array (positions), no interleaving

			vertexBindingDescriptions[0].binding = 0;
			vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexBindingDescriptions[0].stride = sizeof(float) * 2;
		}

		if(renderable->geometry->colors)
		{
			vertextAttributeDescriptions[1].binding = 1;
			vertextAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
			vertextAttributeDescriptions[1].location = 1;
			vertextAttributeDescriptions[1].offset = 0; // because only one type of data in this array (colors), no interleaving

			vertexBindingDescriptions[1].binding = 1;
			vertexBindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexBindingDescriptions[1].stride = sizeof(float) * 3;
		}

		if(renderable->geometry->indices)
		{
			vertextAttributeDescriptions[2].binding = 2;
			vertextAttributeDescriptions[2].format = VK_FORMAT_R16_UINT; // uint16_t
			vertextAttributeDescriptions[2].location = 2;
			vertextAttributeDescriptions[2].offset = 0; // because only one type of data in this array (indices), no interleaving

			vertexBindingDescriptions[2].binding = 2;
			vertexBindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexBindingDescriptions[2].stride = sizeof(uint16_t);
		}

		vertexInputInfo.pVertexAttributeDescriptions = vertextAttributeDescriptions;
		vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescriptions;
	}
	else
	{
		vertexInputInfo.pVertexAttributeDescriptions = NULL;
		vertexInputInfo.pVertexBindingDescriptions = NULL;
	}

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
	colorBlendAttachment.blendEnable = VK_FALSE;
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

	// Required here even when we don't use it
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	if(renderable->uniformDescriptorSetLayout)
	{
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &renderable->uniformDescriptorSetLayout;
	}

	if (vkCreatePipelineLayout(engine.vulkanContext.device, &pipelineLayoutInfo, NULL, &renderable->pipelineLayout))
	{
		fprintf(stderr, "Failed to create pipeline layout");

		goto err_pipeline_layout;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = sizeof shaderStages / sizeof shaderStages[0];
	pipelineInfo.pStages = shaderStages;
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
		vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);
	}
	else
		result = 0;

err_pipeline_layout:
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->fragShader, NULL);
err_frag:
	vkDestroyShaderModule(engine.vulkanContext.device, renderable->vertShader, NULL);
err_vert:

	return result;
}


int createCommandBuffers(renderable renderable)
{
	renderable->commandBuffers = malloc(sizeof * renderable->commandBuffers * engine.vulkanContext.imageCount);

	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = engine.vulkanContext.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = engine.vulkanContext.imageCount;

	VkResult result = vkAllocateCommandBuffers(engine.vulkanContext.device, &allocInfo, renderable->commandBuffers);

	if (result != VK_SUCCESS)
		fprintf(stderr, "Failed to allocate command buffers.\n");

	return result;
}


int updateCommandBuffers(renderable renderable)
{
	VkResult result = VK_SUCCESS;

	if(renderable->needCommandBufferUpdate == VK_FALSE)
		return result;

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = NULL; // NULL in case of primary

	VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassInfo.renderPass = engine.vulkanContext.renderPass;
	// render area : pixels outside have undefined values
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = engine.vulkanContext.swapchainExtent;
	// clear color (see VK_ATTACHMENT_LOAD_OP_CLEAR)
	VkClearColorValue clearColor = {{ 0.2f, 0.2f, 0.2f, 1.0f }};
	VkClearValue clearValue = { clearColor };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	for (uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
	{
		result = vkBeginCommandBuffer(renderable->commandBuffers[i], &beginInfo);

		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "Failed to begin recording command buffer %d/%d.\n", i + 1, engine.vulkanContext.imageCount);

			return result;
		}

		renderPassInfo.framebuffer = engine.vulkanContext.swapChainFramebuffers[i];
		vkCmdBeginRenderPass(renderable->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // VK_SUBPASS_CONTENTS_INLINE for primary
		vkCmdBindPipeline(renderable->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline);

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

			vkCmdBindVertexBuffers(renderable->commandBuffers[i], 0, 2, buffers, offsets);

			if(renderable->geometry->indices)
				vkCmdBindIndexBuffer(renderable->commandBuffers[i], renderable->geometry->bufferIndices, 0, VK_INDEX_TYPE_UINT16);
		}

		// bind descriptor set to send uniforms
		vkCmdBindDescriptorSets(renderable->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineLayout, 0, 1, &renderable->uniformDescriptorSets[i], 0, NULL);

		if(renderable->geometry->indices)
			vkCmdDrawIndexed(renderable->commandBuffers[i], renderable->geometry->vertexCount, 1, 0, 0, 0);
		else
			vkCmdDraw(renderable->commandBuffers[i], renderable->geometry->vertexCount, 1, 0, 0);

		vkCmdEndRenderPass(renderable->commandBuffers[i]);

		result = vkEndCommandBuffer(renderable->commandBuffers[i]);

		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "Failed to end recording command buffer %d/%d.\n", i + 1, engine.vulkanContext.imageCount);

			return result;
		}
	}

	renderable->needCommandBufferUpdate = VK_FALSE;

	return result;
}


int createUniformsBuffer(renderable renderable)
{
	renderable->uniformBuffers = malloc(sizeof * renderable->uniformBuffers * engine.vulkanContext.imageCount);
	renderable->uniformBuffersMemory = malloc(sizeof * renderable->uniformBuffersMemory * engine.vulkanContext.imageCount);

	for(uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
		createBuffer(sizeof(modelViewProj), &renderable->uniformBuffers[i], &renderable->uniformBuffersMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	return 0;
}


int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp)
{
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
	poolSize.descriptorCount = engine.vulkanContext.imageCount;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = &poolSize;
	descriptorPoolInfo.maxSets = engine.vulkanContext.imageCount; // maximum number of descriptor sets that can be allocated from the pool 

	if(vkCreateDescriptorPool(engine.vulkanContext.device, &descriptorPoolInfo, NULL, &renderable->uniformDescriptorPool))
	{
		fprintf(stderr, "vkCreateDescriptorPool() failed)\n");

		return -1;
	}

	return 0;
}


int createDescriptorSets(renderable renderable)
{
	VkDescriptorSetLayout *layouts = malloc(sizeof * layouts * engine.vulkanContext.imageCount);

	for(uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
		layouts[i] = renderable->uniformDescriptorSetLayout;

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = renderable->uniformDescriptorPool;
	allocInfo.pSetLayouts = layouts;
	allocInfo.descriptorSetCount = engine.vulkanContext.imageCount;

	renderable->uniformDescriptorSets = malloc(sizeof *renderable->uniformDescriptorSets * engine.vulkanContext.imageCount);

	VkResult result = vkAllocateDescriptorSets(engine.vulkanContext.device, &allocInfo, renderable->uniformDescriptorSets);
	free(layouts);

	if(result)
	{
		fprintf(stderr, "vkAllocateDescriptorSets() failed)\n");

		return -1;
	}

	for(uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
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
