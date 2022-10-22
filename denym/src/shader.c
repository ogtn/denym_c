#include "shader.h"
#include "core.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


shader shaderCreate(VkDevice device, const char* name)
{
	FILE* f;
	char fullName[FILENAME_MAX];

	snprintf(fullName, FILENAME_MAX, "resources/shaders/%s", name);
	shader shader = resourceCacheGet(engine.caches.shaderCache, fullName);

	if(shader != NULL)
		return shader;

#ifdef _MSC_VER
	fopen_s(&f, fullName, "rb");
#else
	f = fopen(fullName, "r");
#endif

	if (f == NULL)
	{
		logError("Failed to open file '%s'", fullName);

		return shader;
	}

	fseek(f, 0, SEEK_END);
	size_t size = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);

	uint32_t* data = malloc(size);

	if (fread(data, 1, size, f) != size)
		perror("");

	fclose(f);

	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = size;
	createInfo.pCode = data;

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);

	if(result)
		logError("Failed to load shader '%s'", fullName);
	else
	{
		shader = malloc(sizeof *shader);
		strncpy(shader->name, fullName, sizeof shader->name);
		shader->shaderModule = shaderModule;
		resourceCacheAdd(engine.caches.shaderCache, fullName, shader);
	}

	free(data);

	return shader;
}


void shaderDestroy(shader shader)
{
	VkBool32 needDestruction;

	resourceCacheRemove(engine.caches.shaderCache, shader->name, &needDestruction);

	if(needDestruction)
		vkDestroyShaderModule(engine.vulkanContext.device, shader->shaderModule, NULL);
}
