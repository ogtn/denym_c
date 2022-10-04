#include "shader.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>


int loadShader(VkDevice device, const char* name, VkShaderModule* outShaderr)
{
	FILE* f;
	char fullName[FILENAME_MAX];

	snprintf(fullName, FILENAME_MAX, "resources/shaders/%s", name);

#ifdef _MSC_VER
	fopen_s(&f, fullName, "rb");
#else
	f = fopen(fullName, "r");
#endif

	if (f == NULL)
	{
		logError("Failed to open file '%s'", fullName);

		return -1;
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

	VkResult result = vkCreateShaderModule(device, &createInfo, NULL, outShaderr);

	if(result)
		logError("Failed to load shader '%s'", fullName);

	free(data);

	return result;
}
