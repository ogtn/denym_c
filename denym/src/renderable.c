#include "renderable.h"
#include "geometry.h"
#include "shader.h"
#include "core.h"

#include <stdlib.h>
#include <stdio.h>


static renderable_t static_renderable;


renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName)
{
	renderable renderable = &static_renderable;

	renderable->vertShaderName = vertShaderName;
	renderable->fragShaderName = fragShaderName;
	renderable->geometry = geometry;

	if(!createPipeline(renderable) && !createBuffers(geometry))
	{
		return renderable;
	}

	return NULL;
}


void denymDestroyRenderable(renderable renderable)
{
	// TODO clean a shitload of stuff here !!! (shaders...)

	denymDestroyGeometry(renderable->geometry);
	vkDestroyPipeline(engine.vulkanContext.device, renderable->pipeline, NULL);
	vkDestroyPipelineLayout(engine.vulkanContext.device, renderable->pipelineLayout, NULL);
	vkFreeCommandBuffers(engine.vulkanContext.device, engine.vulkanContext.commandPool, engine.vulkanContext.imageCount, renderable->commandBuffers);
	free(renderable->commandBuffers);
}


int createPipeline(renderable renderable)
{
	int result = -1;
	VkShaderModule vertShader;
	VkShaderModule fragShader;

	if (loadShader(engine.vulkanContext.device, renderable->vertShaderName, &vertShader))
		goto err_vert;

	if (loadShader(engine.vulkanContext.device, renderable->fragShaderName, &fragShader))
		goto err_frag;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShader;
	vertShaderStageInfo.pName = "main";
	// TODO : check vertShaderStageInfo.pSpecializationInfo to pass constants to the shaders

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShader;
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

	// Required here even if we don't need it (hence empty). Use to set uniforms in shaders
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

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
	vkDestroyShaderModule(engine.vulkanContext.device, fragShader, NULL);
err_frag:
	vkDestroyShaderModule(engine.vulkanContext.device, vertShader, NULL);
err_vert:

	return result;
}
