#ifdef _MSC_VER
#include <windows.h>
void usleep(__int64 usec);
#else
#include <unistd.h>
#endif


#include "denym_private.h"
#include <stdio.h>
#include <string.h>


static denym engine;


int denymInit(int window_width, int window_height)
{
	int result = -1;

	if ((engine.window = createWindow(window_width, window_height)) != NULL &&
		!createVulkanInstance(&engine.vulkanContext) &&
		!glfwCreateWindowSurface(engine.vulkanContext.instance, engine.window, NULL, &engine.vulkanContext.surface) &&
		!getPhysicalDevice(&engine.vulkanContext) &&
		!getDevice(&engine.vulkanContext) &&
		!getDeviceExtensionsAddr(&engine.vulkanContext) &&
		!getSwapchainCapabilities(&engine.vulkanContext) &&
		!createSwapchain(&engine.vulkanContext) &&
		!createImageViews(&engine.vulkanContext) &&
		!createRenderPass(&engine.vulkanContext) &&
		!createPipeline(&engine.vulkanContext) &&
		!createFramebuffer(&engine.vulkanContext) &&
		!createCommandPool(&engine.vulkanContext) &&
		!createCommandBuffers(&engine.vulkanContext) &&
		!createSynchronizationObjects(&engine.vulkanContext))
	{
		result = 0;
        engine.vulkanContext.currentFrame = 0;
	}
	else
	{
		denymTerminate();
	}

	return result;
}


void denymTerminate(void)
{
	vkDeviceWaitIdle(engine.vulkanContext.device);
	destroyVulkanContext(&engine.vulkanContext);
	glfwDestroyWindow(engine.window);
	glfwTerminate();

	memset(&engine.vulkanContext, 0, sizeof(vulkanContext));
	memset(&engine, 0, sizeof(denym));
}


int denymKeepRunning(void)
{
	glfwPollEvents();

	return !glfwWindowShouldClose(engine.window) && !glfwGetKey(engine.window, GLFW_KEY_ESCAPE);
}


void denymRender(void)
{
	render(&engine.vulkanContext);
	engine.vulkanContext.currentFrame = (engine.vulkanContext.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void denymWaitForNextFrame(void)
{
	usleep(2000);
}


void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW error %d occured : %s\n", error, description);
}


GLFWwindow* createWindow(int width, int height)
{
	GLFWwindow* window;
	const GLFWvidmode* videoMode;
	GLFWmonitor* monitor;

	if (!glfwInit())
	{
		fprintf(stderr, "Holy fuckin' shit, glfwInit() failed\n");

		return NULL;
	}

	glfwSetErrorCallback(glfwErrorCallback);

	if (!glfwVulkanSupported())
	{
		fprintf(stderr, "Vulkan is not suported :(\n");

		return NULL;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, APP_NAME, NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "Holy fuckin' shit, glfwCreateWindow() failed\n");
		glfwTerminate();

		return NULL;
	}

	// center the window
	monitor = glfwGetPrimaryMonitor();
	videoMode = glfwGetVideoMode(monitor);
	glfwSetWindowPos(window, (videoMode->width - width) / 2, (videoMode->height - height) / 2);

	return window;
}


int createVulkanInstance(vulkanContext* context)
{
	uint32_t count;

	// List available layers
	if (!vkEnumerateInstanceLayerProperties(&count, NULL))
	{
		VkLayerProperties* availableLayers = malloc(sizeof * availableLayers * count);
		vkEnumerateInstanceLayerProperties(&count, availableLayers);

		fprintf(stderr, "%d instance layers properties available : \n", count);

		for (uint32_t i = 0; i < count; i++)
			fprintf(stderr, "  %s : %s\n", availableLayers[i].layerName, availableLayers[i].description);

		free(availableLayers);
	}

	// List available extensions
	VkExtensionProperties* availableExtensions = NULL;
	const char** requestedExtensions = NULL;
	uint32_t requestedExtensionCount = 0;

	if (!vkEnumerateInstanceExtensionProperties(NULL, &count, NULL))
	{
		availableExtensions = malloc(sizeof * availableExtensions * count);
		requestedExtensions = malloc(sizeof * requestedExtensions * count);
		vkEnumerateInstanceExtensionProperties(NULL, &count, availableExtensions);

		fprintf(stderr, "%d Vulkan extensions available : \n", count);

		for (uint32_t i = 0; i < count; i++)
		{
#if defined(_DEBUG)
			if (!strcmp(availableExtensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
				requestedExtensions[requestedExtensionCount++] = availableExtensions[i].extensionName;
#endif

			fprintf(stderr, "  %s\n", availableExtensions[i].extensionName);
		}
	}

	// Get extensions required by GLFW
	uint32_t requiredExtensionsCount;
	const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

	fprintf(stderr, "%d Vulkan extensions required by GLFW :\n", requiredExtensionsCount);

	for (uint32_t i = 0; i < requiredExtensionsCount; i++)
	{
		int available = 0;

		for (uint32_t j = 0; j < count; j++)
		{
			if (!strcmp(requiredExtensions[i], availableExtensions[j].extensionName))
			{
				requestedExtensions[requestedExtensionCount++] = requiredExtensions[i];
				available = 1;
			}
		}

		fprintf(stderr, "  %d : %s => %s\n", i, requiredExtensions[i], available ? "available" : "missing");
	}

	// Set application information
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = APP_NAME;
	appInfo.applicationVersion = APP_VERSION;
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = ENGINE_VERSION;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// Configure debug logger callback
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugMessengerCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | // too verbose, but could be usefull
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = vulkanErrorCallback;

	// Set instance information
	VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instInfo.pNext = &debugMessengerCreateInfo;
	instInfo.pApplicationInfo = &appInfo;
	instInfo.enabledExtensionCount = requestedExtensionCount;
	instInfo.ppEnabledExtensionNames = requestedExtensions;
	// Use validation layers if this is a debug build
#if defined(_DEBUG)
	const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
	instInfo.enabledLayerCount = sizeof(layers) / sizeof layers[0];
	instInfo.ppEnabledLayerNames = layers;
#endif

	// Create the Vulkan instance.
	VkResult result = vkCreateInstance(&instInfo, NULL, &context->instance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		fprintf(stderr, "Unable to find a compatible Vulkan Driver.\n");
	else if (result)
		fprintf(stderr, "Could not create a Vulkan instance (for unknown reasons).");

	free((void*)requestedExtensions); // cast to avoid C4090 warning from MSVC
	free(availableExtensions);

	// TODO: check return value
	getInstanceExtensionsAddr(context);

	// TODO: check return value
	context->CreateDebugUtilsMessengerEXT(context->instance, &debugMessengerCreateInfo, NULL, &context->messenger);

	return result;
}


int getPhysicalDevice(vulkanContext* context)
{
	uint32_t count;
	VkPhysicalDevice matchingPhysicalDevice = NULL;

	vkEnumeratePhysicalDevices(context->instance, &count, NULL);
	fprintf(stderr, "%d physical devices found :\n", count);
	VkPhysicalDevice* physicalDevices = malloc(sizeof * physicalDevices * count);
	vkEnumeratePhysicalDevices(context->instance, &count, physicalDevices);

	for (uint32_t i = 0; i < count; i++)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
		fprintf(stderr, "  %d : '%s' :\n", i, physicalDeviceProperties.deviceName);

		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			matchingPhysicalDevice = physicalDevices[i];
			context->physicalDeviceProperties = physicalDeviceProperties;
			break;
		}
	}

	if (matchingPhysicalDevice == NULL)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			{
				matchingPhysicalDevice = physicalDevices[i];
				context->physicalDeviceProperties = physicalDeviceProperties;
				break;
			}
		}
	}

	free(physicalDevices);

	if (matchingPhysicalDevice == NULL)
		return -1;

	context->physicalDevice = matchingPhysicalDevice;
	getPhysicalDeviceCapabilities(context);

	return 0;
}


void getPhysicalDeviceCapabilities(vulkanContext* context)
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, NULL);
	fprintf(stderr, "    %d queue family found:\n", queueFamilyCount);
	VkQueueFamilyProperties* queueFamilyProperties = malloc(sizeof * queueFamilyProperties * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, queueFamilyProperties);

	context->graphicsQueueFamilyIndex = -1;
	context->presentQueueFamilyIndex = -1;

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		int canPresentImages = glfwGetPhysicalDevicePresentationSupport(context->instance, context->physicalDevice, i);

		fprintf(stderr, "      queue family %d :\n", i);
		fprintf(stderr, "        queueFlags : %#010x.\n", queueFamilyProperties[i].queueFlags);
		fprintf(stderr, "        queueCount : %d.\n", queueFamilyProperties[i].queueCount);
		fprintf(stderr, "        can present images : %s.\n", canPresentImages ? "true" : "false");

		if (context->graphicsQueueFamilyIndex == -1 &&
			queueFamilyProperties[i].queueCount >= 1 &&
			queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			context->graphicsQueueFamilyIndex = i;

		if (context->presentQueueFamilyIndex == -1 &&
			queueFamilyProperties[i].queueCount >= 1 &&
			canPresentImages)
			context->presentQueueFamilyIndex = i;
	}

	free(queueFamilyProperties);

	vkGetPhysicalDeviceFeatures(context->physicalDevice, &context->physicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &context->physicalDeviceMemoryProperties);
}


int getDevice(vulkanContext *context)
{
	uint32_t count;

	// List available layers
	if (!vkEnumerateDeviceLayerProperties(context->physicalDevice, &count, NULL))
	{
		VkLayerProperties* availableLayers = malloc(sizeof * availableLayers * count);
		vkEnumerateDeviceLayerProperties(context->physicalDevice, &count, availableLayers);

		fprintf(stderr, "%d device layer properties available : \n", count);

		for (uint32_t i = 0; i < count; i++)
			fprintf(stderr, "  %s : %s\n", availableLayers[i].layerName, availableLayers[i].description);

		free(availableLayers);
	}

	// List available extensions
	if (!vkEnumerateDeviceExtensionProperties(context->physicalDevice, NULL, &count, NULL))
	{
		VkExtensionProperties* availableExtensions = malloc(sizeof * availableExtensions * count);
		vkEnumerateDeviceExtensionProperties(context->physicalDevice, NULL, &count, availableExtensions);

		fprintf(stderr, "%d device extensions available : \n", count);

		for (uint32_t i = 0; i < count; i++)
			fprintf(stderr, "  %s\n", availableExtensions[i].extensionName);

		free(availableExtensions);
	}

	float queue_priorities[1] = { 1 };
	VkDeviceQueueCreateInfo queues[2] = { { 0 } };
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueFamilyIndex = context->graphicsQueueFamilyIndex;
	queues[0].queueCount = 1;
	queues[0].pQueuePriorities = queue_priorities;

	queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[1].queueFamilyIndex = context->presentQueueFamilyIndex;
	queues[1].queueCount = 1;
	queues[1].pQueuePriorities = queue_priorities;

	const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.queueCreateInfoCount = context->graphicsQueueFamilyIndex == context->presentQueueFamilyIndex ? 1 : 2,
	deviceInfo.pQueueCreateInfos = queues,
	deviceInfo.enabledExtensionCount = sizeof extensions / sizeof extensions[0];
	deviceInfo.ppEnabledExtensionNames = extensions;
	// Use validation layers if this is a debug build
#if defined(_DEBUG)
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	deviceInfo.enabledLayerCount = sizeof(layers) / sizeof layers[0];
	deviceInfo.ppEnabledLayerNames = layers;
#endif

	VkResult result = vkCreateDevice(context->physicalDevice, &deviceInfo, NULL, &context->device);

	if(result == VK_SUCCESS)
	{
		vkGetDeviceQueue(context->device, context->graphicsQueueFamilyIndex, 0, &context->graphicQueue);
		vkGetDeviceQueue(context->device, context->presentQueueFamilyIndex, 0, &context->presentQueue);
	}
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		fprintf(stderr, "Could not create device : missing requested extension(s).\n");
	else if( result == VK_ERROR_FEATURE_NOT_PRESENT)
		fprintf(stderr, "Could not create device : missing requested feature(s).\n");
	else
		fprintf(stderr, "Could not create device : unknow error.\n");

	return result;
}


#define getInstanceProcAddr(vulkanContext, name)                                                                            \
	do {                                                                                                                    \
		if((vulkanContext->name = (PFN_vk##name)glfwGetInstanceProcAddress(vulkanContext->instance, "vk" #name)) == NULL)   \
		{                                                                                                                   \
			fprintf(stderr, "Failed to get 'vk" #name " pointer from loader'\n");                                           \
			res = -1;                                                                                                       \
		}                                                                                                                   \
	} while(0)                                                                                                              \


#define getDeviceProcAddr(vulkanContext, name)                                                                                  \
	do {                                                                                                                        \
		if((vulkanContext->name = (PFN_vk##name)vulkanContext->GetDeviceProcAddr(vulkanContext->device, "vk" #name)) == NULL)   \
		{                                                                                                                       \
			fprintf(stderr, "Failed to get 'vk" #name " pointer from loader'\n");                                               \
			res = -1;                                                                                                           \
		}                                                                                                                       \
	} while(0)                                                                                                                  \


int getInstanceExtensionsAddr(vulkanContext* context)
{
	int res = 0;

	getInstanceProcAddr(context, GetDeviceProcAddr);
	getInstanceProcAddr(context, GetPhysicalDeviceSurfaceSupportKHR);
	getInstanceProcAddr(context, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	getInstanceProcAddr(context, GetPhysicalDeviceSurfaceFormatsKHR);
	getInstanceProcAddr(context, GetPhysicalDeviceSurfacePresentModesKHR);
	getInstanceProcAddr(context, CreateDebugUtilsMessengerEXT);
	getInstanceProcAddr(context, DestroyDebugUtilsMessengerEXT);

	return res;
}


int getDeviceExtensionsAddr(vulkanContext* context)
{
	int res = 0;

	getDeviceProcAddr(context, DestroySurfaceKHR);
	getDeviceProcAddr(context, CreateSwapchainKHR);
	getDeviceProcAddr(context, DestroySwapchainKHR);
	getDeviceProcAddr(context, GetSwapchainImagesKHR);
	getDeviceProcAddr(context, AcquireNextImageKHR);
	getDeviceProcAddr(context, QueuePresentKHR);
	getDeviceProcAddr(context, SetDebugUtilsObjectNameEXT);

	return 0;
}


int getSwapchainCapabilities(vulkanContext* context)
{
	// TODO: check that shit, this must be verified before creating the queues...
	VkBool32 supported;
	context->GetPhysicalDeviceSurfaceSupportKHR(context->physicalDevice, context->presentQueueFamilyIndex, context->surface, &supported);

	context->GetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &context->surfaceCapabilities);

	uint32_t formatCount;
	context->GetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, NULL);
	VkSurfaceFormatKHR* formats = malloc(sizeof * formats * formatCount);
	context->GetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, formats);

	fprintf(stderr, "%d format supported\n", formatCount);
	int formatFound = 0;

	for (uint32_t i = 0; i < formatCount; i++)
	{
		if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && 
			formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			fprintf(stderr, "Found : %#010x Color space : %#010x\n", formats[i].format, formats[i].colorSpace);
			context->surfaceFormat = formats[i];
			formatFound = 1;
			break;
		}
	}

	if (!formatFound)
	{
		fprintf(stderr, "No suitable format found.\n");
		return -1;
	}

	uint32_t presentModeCount;
	context->GetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &presentModeCount, NULL);
	VkPresentModeKHR* presentModes = malloc(sizeof * presentModes * presentModeCount);
	context->GetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &presentModeCount, presentModes);

	fprintf(stderr, "%d present modes supported :\n", presentModeCount);

	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		// Triple bufferering if possible
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			context->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		// Double buffering, that one is always available
		else if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
			context->presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	fprintf(stderr, "Found present mode : %#010x\n", context->presentMode);

	free(formats);
	free(presentModes);

	return formatCount == 0 || presentModeCount == 0;
}


int createSwapchain(vulkanContext* context)
{
	// TODO: case max = 0 for unlimited
	uint32_t imageCount = clamp(
		context->surfaceCapabilities.minImageCount + 1,
		context->surfaceCapabilities.minImageCount,
		context->surfaceCapabilities.maxImageCount);

	// need to clamp here, as current can be 0xffffffff
	context->swapchainExtent = clampExtent2D(
		context->surfaceCapabilities.currentExtent,
		context->surfaceCapabilities.minImageExtent,
		context->surfaceCapabilities.maxImageExtent);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = context->surface;
	createInfo.imageColorSpace = context->surfaceFormat.colorSpace;
	createInfo.imageExtent = context->swapchainExtent;
	createInfo.imageArrayLayers = 1; // more than one is for stereoscopic
	createInfo.imageFormat = context->surfaceFormat.format;
	createInfo.minImageCount = imageCount;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = context->presentMode;
	createInfo.preTransform = context->surfaceCapabilities.currentTransform;
	createInfo.clipped = VK_TRUE;

	// If the two queues are the same
	if(context->graphicsQueueFamilyIndex == context->presentQueueFamilyIndex)
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		// we need a bit more work as the image needs to be shared by two distinct queues
		uint32_t queueFamilyIndices[2];

		queueFamilyIndices[0] = context->graphicsQueueFamilyIndex;
		queueFamilyIndices[1] = context->presentQueueFamilyIndex;

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = sizeof queueFamilyIndices / sizeof *queueFamilyIndices;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	VkResult result = context->CreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapchain);

	if (result != VK_SUCCESS)
		fprintf(stderr, "Swapchain creation failed.\n");

	return result;
}

int createImageViews(vulkanContext* context)
{
	context->GetSwapchainImagesKHR(context->device, context->swapchain, &context->imageCount, NULL);
	context->images = malloc(sizeof *context->images * context->imageCount);
	context->imageViews = malloc(sizeof *context->imageViews * context->imageCount);
	context->GetSwapchainImagesKHR(context->device, context->swapchain, &context->imageCount, context->images);

	for (uint32_t i = 0; i < context->imageCount; i++)
	{
		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = context->images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = context->surfaceFormat.format;
		// unnecessary for IDENTITY, but better be explicit to remember this feature
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(context->device, &createInfo, NULL, &context->imageViews[i]))
		{
			fprintf(stderr, "Failed to create image view.\n");

			return -1;
		}
	}

	return 0;
}


int loadShader(vulkanContext* context, const char* name, VkShaderModule* outShaderr)
{
	FILE* f;

#ifdef _MSC_VER
	fopen_s(&f, name, "rb");
#else
	f = fopen(name, "r");
#endif

	if (f == NULL)
	{
		perror(name);

		return -1;
	}

	fseek(f, 0, SEEK_END);
	uint32_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint32_t* data = malloc(size);

	if (fread(data, 1, size, f) != size)
		perror("");

	fclose(f);

	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = size;
	createInfo.pCode = data;

	VkResult result = vkCreateShaderModule(context->device, &createInfo, NULL, outShaderr);

	if(result)
		fprintf(stderr, "Failed to load shader \"%s\"", name);

	free(data);

	return result;
}


int createRenderPass(vulkanContext* context)
{
	VkAttachmentDescription colorAttachment = { 0 };
	colorAttachment.format = context->surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling => 1 sample
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear framebuffer between each rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // write to the framebuffer when rendering
	// no stencil buffer
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// layout before and after rendering
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // don't care, we'll clear it anyway
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout suitable for present in swapchain

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0; // index of the attachment (only one in our case)
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// first and single subpass, containing one single color attachment reference, referencing our unique attachment
	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics, no GPGPU
	subpass.colorAttachmentCount = 1;
	// index matching our fragment shader output: layout(location = 0) out vec4 outColor
	subpass.pColorAttachments = &colorAttachmentRef;

	// Implicit dependency
	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	// list dependancies in case of multiple passes
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(context->device, &renderPassInfo, NULL, &context->renderPass);

	if (result)
		fprintf(stderr, "Error while creating renderpass.\n");

	return result;
}


int createPipeline(vulkanContext* context)
{
	int result = -1;
	VkShaderModule vertShader;
	VkShaderModule fragShader;

	if (loadShader(context, "resources/shaders/hardcoded_triangle.vert.spv", &vertShader))
		goto err_vert;

	if (loadShader(context, "resources/shaders/basic_color_interp.frag.spv", &fragShader))
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

	// empty in our case, as we don't send vertex attributes
	// everything is hardcoded in the shaders
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = NULL;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = NULL;

	// type of geometry we want to draw
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewport taking all the space available
	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)context->swapchainExtent.width;
	viewport.height = (float)context->swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// no scissoring
	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = context->swapchainExtent;

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

	if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &context->pipelineLayout))
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
	pipelineInfo.pDynamicState = NULL; // &dynamicState;
	pipelineInfo.layout = context->pipelineLayout;
	pipelineInfo.renderPass = context->renderPass;
	pipelineInfo.subpass = 0; // index of our only subpass
	// those two are needed only when re-using another existing pipeline
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE ;
	pipelineInfo.basePipelineIndex = -1;

	// interesting second parameter : pipeline cache can be used to speedup all this, even across runs, through a file !
	if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &context->pipeline))
	{
		fprintf(stderr, "Failed to create graphic pipeline.\n");
		vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
	}
	else
		result = 0;

err_pipeline_layout:
	vkDestroyShaderModule(context->device, fragShader, NULL);
err_frag:
	vkDestroyShaderModule(context->device, vertShader, NULL);
err_vert:

	return result;
}


int createFramebuffer(vulkanContext* context)
{
	context->swapChainFramebuffers = malloc(sizeof * context->swapChainFramebuffers * context->imageCount);

	VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = context->renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = context->swapchainExtent.width;
	framebufferInfo.height = context->swapchainExtent.height;
	framebufferInfo.layers = 1;

	for (uint32_t i = 0; i < context->imageCount; i++)
	{
		framebufferInfo.pAttachments = &context->imageViews[i];
		VkResult result = vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &context->swapChainFramebuffers[i]);

		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "Failed to create framebuffer %d/%d.\n", i + 1, context->imageCount);

			return result;
		}
	}

	return VK_SUCCESS;
}


int createCommandPool(vulkanContext* context)
{
	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = context->graphicsQueueFamilyIndex;
	poolInfo.flags = 0; // we don't care for this small example, with a fully static scene

	VkResult result = vkCreateCommandPool(context->device, &poolInfo, NULL, &context->commandPool);

	if (result != VK_SUCCESS)
		fprintf(stderr, "Error while creating the command pool.\n");

	return result;
}


int createCommandBuffers(vulkanContext* context)
{
	context->commandBuffers = malloc(sizeof * context->commandBuffers * context->imageCount);

	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = context->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = context->imageCount;

	VkResult result = vkAllocateCommandBuffers(context->device, &allocInfo, context->commandBuffers);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "Failed to allocate command buffers.\n");

		return result;
	}

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = NULL; // NULL in case of primary

	VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassInfo.renderPass = context->renderPass;
	// render area : pixels outside have undefined values
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = context->swapchainExtent;
	// clear color (see VK_ATTACHMENT_LOAD_OP_CLEAR)
	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	for (uint32_t i = 0; i < context->imageCount; i++)
	{
		result = vkBeginCommandBuffer(context->commandBuffers[i], &beginInfo);

		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "Failed to begin recording command buffer %d/%d.\n", i + 1, context->imageCount);

			return result;
		}

		renderPassInfo.framebuffer = context->swapChainFramebuffers[i];
		vkCmdBeginRenderPass(context->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // VK_SUBPASS_CONTENTS_INLINE for primary
		vkCmdBindPipeline(context->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline);
		vkCmdDraw(context->commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(context->commandBuffers[i]);

		result = vkEndCommandBuffer(context->commandBuffers[i]);

		if (result != VK_SUCCESS)
		{
			fprintf(stderr, "Failed to end recording command buffer %d/%d.\n", i + 1, context->imageCount);

			return result;
		}
	}

	return result;
}


int createSynchronizationObjects(vulkanContext* context)
{
	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // fence is not blocking when created

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->imageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->renderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, NULL, &context->inFlightFences[i]))
        {
            fprintf(stderr, "Failed to create sync objets.\n");

            return -1;
        }

		/*
		#ifdef _DEBUG
		VkDebugUtilsObjectNameInfoEXT objectNameInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		objectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;

		objectNameInfo.pObjectName = "Image available semaphore";
		objectNameInfo.objectHandle = (uint64_t)context->imageAvailableSemaphore;
		context->SetDebugUtilsObjectNameEXT(context->device, &objectNameInfo);

		objectNameInfo.pObjectName = "Render finished semaphore";
		objectNameInfo.objectHandle = (uint64_t)context->renderFinishedSemaphore;
		context->SetDebugUtilsObjectNameEXT(context->device, &objectNameInfo);
		#endif
		*/
    }

	return 0;
}


void render(vulkanContext *context)
{
	uint32_t imageIndex;

    // Wait for fence, so we limit the number of in flight frames
    vkWaitForFences(context->device, 1, &context->inFlightFences[context->currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->device, 1, &context->inFlightFences[context->currentFrame]);

    VkResult result = context->AcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->imageAvailableSemaphore[context->currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result)
        result = 0;

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

	// one wait stage (color writing, and it's associated semaphore). Could be more stages here
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &context->imageAvailableSemaphore[context->currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	// command buffer associated with the image available
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context->commandBuffers[imageIndex];
	// semaphore to signal when the command buffer's execution is finished
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &context->renderFinishedSemaphore[context->currentFrame];

	result = vkQueueSubmit(context->graphicQueue, 1, &submitInfo, context->inFlightFences[context->currentFrame]);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "Failed to submit draw command buffer.\n");
	}

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &context->renderFinishedSemaphore[context->currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context->swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = &result; // Optional when only one swapchain, provide an array in case of multiple swapchains

    result = context->QueuePresentKHR(context->presentQueue, &presentInfo);

    if (result)
        result = 0;
}


uint32_t clamp(uint32_t n, uint32_t min, uint32_t max)
{
	if (n < min)
		return min;

	if (n > max)
		return max;

	return n;
}


VkExtent2D clampExtent2D(VkExtent2D e, VkExtent2D min, VkExtent2D max)
{
	VkExtent2D res;

	res.height = clamp(e.height, min.height, max.height);
	res.width = clamp(e.width, min.width, max.width);

	return res;
}


void destroyVulkanContext(vulkanContext* context)
{
	if (context->device)
	{
        cleanSwapchain(context);
		vkDestroyCommandPool(context->device, context->commandPool, NULL);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(context->device, context->imageAvailableSemaphore[i], NULL);
            vkDestroySemaphore(context->device, context->renderFinishedSemaphore[i], NULL);
            vkDestroyFence(context->device, context->inFlightFences[i], NULL);
        }

		vkDestroyDevice(context->device, NULL);
	}

	if (context->instance)
	{
		context->DestroySurfaceKHR(context->instance, context->surface, NULL);
		context->DestroyDebugUtilsMessengerEXT(context->instance, context->messenger, NULL);
		vkDestroyInstance(context->instance, NULL);
	}
}


void cleanSwapchain(vulkanContext* context)
{
    vkDestroyPipeline(context->device, context->pipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);
    vkDestroyRenderPass(context->device, context->renderPass, NULL);
    vkFreeCommandBuffers(context->device, context->commandPool, context->imageCount, context->commandBuffers);

    for (uint32_t i = 0; i < context->imageCount; i++)
    {
        vkDestroyImageView(context->device, context->imageViews[i], NULL);
        vkDestroyFramebuffer(context->device, context->swapChainFramebuffers[i], NULL);
    }

    context->DestroySwapchainKHR(context->device, context->swapchain, NULL);
    free(context->images);
    free(context->commandBuffers);
    free(context->imageViews);
    free(context->swapChainFramebuffers);
}


VKAPI_ATTR VkBool32 VKAPI_CALL vulkanErrorCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	messageSeverity;
	messageType;
	pUserData;
	fprintf(stderr, "Vulkan log : %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}


#ifdef _MSC_VER
// https://www.c-plusplus.net/forum/topic/109539/usleep-unter-windows
void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#endif
