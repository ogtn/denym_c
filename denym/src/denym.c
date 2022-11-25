#include "denym_private.h"
#include "image.h"
#include "texture.h"
#include "utils.h"
#include "scene.h"
#include "camera.h"
#include "logger.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


denym engine;


int denymInit(int window_width, int window_height)
{
	int result = -1;

	timespec_get(&engine.metrics.time.startTime, TIME_UTC);

	// TODO make this configurable
	engine.settings.useMSAA = VK_TRUE;
	engine.settings.useDepthBuffer = VK_TRUE;
	engine.settings.cacheUniformMemory = VK_TRUE;
	engine.settings.cacheStorageBufferMemory = VK_TRUE;

	if ((engine.window = createWindow(window_width, window_height)) != NULL &&
		!inputCreate() &&
		!createVulkanInstance(&engine.vulkanContext) &&
		!glfwCreateWindowSurface(engine.vulkanContext.instance, engine.window, NULL, &engine.vulkanContext.surface) &&
		!getPhysicalDevice(&engine.vulkanContext) &&
		!getDevice(&engine.vulkanContext) &&
		!getDeviceExtensionsAddr(&engine.vulkanContext) &&
		!getSwapchainCapabilities(&engine.vulkanContext) &&
		!createSwapchain(&engine.vulkanContext) &&
		!createImageViews(&engine.vulkanContext) &&
		!createColorResources() &&
		!createDepthBufferResources() &&
		!createRenderPass(&engine.vulkanContext) &&
		!createFramebuffer(&engine.vulkanContext) &&
		!createCommandPool(&engine.vulkanContext) &&
		!createCommandBuffers() &&
		!createSynchronizationObjects(&engine.vulkanContext) &&
		!createTextureSamplers() &&
		!createCaches() &&
		!textureCreate("missing.png", &engine.textureFallback))
	{
		result = 0;
		engine.vulkanContext.currentFrame = 0;
		engine.scene = sceneCreate();

		for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			engine.vulkanContext.needCommandBufferUpdate[i] = VK_TRUE;

		float now = getUptime();

		engine.metrics.time.currentFrame = now;
		engine.metrics.time.lastFrame = now;
		engine.metrics.time.frameWindowStart = now;
	}
	else
	{
		denymTerminate();
	}

	return result;
}


void denymTerminate(void)
{
	sceneDestroy(engine.scene);
	textureDestroy(engine.textureFallback);
	destroyCaches();
	destroyVulkanContext(&engine.vulkanContext);
	glfwDestroyWindow(engine.window);
	glfwTerminate();
	memset(&engine, 0, sizeof(denym));
}


int denymKeepRunning(void)
{
	return !glfwWindowShouldClose(engine.window) &&
		!glfwGetKey(engine.window, GLFW_KEY_ESCAPE) &&
		!engine.input.controller.buttons.home;
}


void denymRender(void)
{
	engine.metrics.time.currentFrame = getUptime();
	engine.metrics.time.sinceLastFrame = engine.metrics.time.currentFrame - engine.metrics.time.lastFrame;

	// Wait for fence, so we limit the number of in flight frames
	vkWaitForFences(engine.vulkanContext.device, 1, &engine.vulkanContext.inFlightFences[engine.vulkanContext.currentFrame], VK_TRUE, UINT64_MAX);
	inputUpdate();
	updateCamera();
	sceneUpdate(engine.scene);
	updateCommandBuffers(engine.vulkanContext.currentFrame);
	render(&engine.vulkanContext);
	engine.vulkanContext.currentFrame = (engine.vulkanContext.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	updateMetrics();
}


void denymWaitForNextFrame(void)
{
	float target = 1.f / 60.f;
	//denymSleep(target - engine.metrics.time.lastRenderTime);
}


void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	engine.vulkanContext.framebufferResized = VK_TRUE;
	engine.framebufferWidth = width;
	engine.framebufferHeight = height;

	cameraResize(sceneGetCamera(engine.scene), width, height);
}


GLFWwindow* createWindow(int width, int height)
{
	GLFWwindow* window;
	const GLFWvidmode* videoMode;
	GLFWmonitor* monitor;

	engine.framebufferWidth = width;
	engine.framebufferHeight = height;

	if (!glfwInit())
	{
		logError("Holy fuckin' shit, glfwInit() failed");

		return NULL;
	}

	glfwSetErrorCallback(glfwErrorCallback);

	if (!glfwVulkanSupported())
	{
		logError("Vulkan is not suported :(");

		return NULL;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, APP_NAME, NULL, NULL);

	if (!window)
	{
		logError("Holy fuckin' shit, glfwCreateWindow() failed");
		glfwTerminate();

		return NULL;
	}

	// center the window
	monitor = glfwGetPrimaryMonitor();
	videoMode = glfwGetVideoMode(monitor);
	glfwSetWindowPos(window, (videoMode->width - width) / 2, (videoMode->height - height) / 2);

	// set callbacks
	glfwSetFramebufferSizeCallback(window, glfwFramebufferResizeCallback);

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
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | // too verbose, but could be usefull
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
		logError("Unable to find a compatible Vulkan Driver.");
	else if (result)
		logError("Could not create a Vulkan instance (reason : %d)", result);

	free((void*)requestedExtensions); // cast to avoid C4090 warning from MSVC
	free(availableExtensions);

	// TODO: check return value
	getInstanceExtensionsAddr(context);

	// TODO: check return value
	//context->CreateDebugUtilsMessengerEXT(context->instance, &debugMessengerCreateInfo, NULL, &context->messenger);

	return result;
}


int getPhysicalDevice(vulkanContext* context)
{
	uint32_t count;
	VkPhysicalDevice matchingPhysicalDevice = NULL;
	VkPhysicalDeviceProperties physicalDeviceProperties;

	vkEnumeratePhysicalDevices(context->instance, &count, NULL);
	logInfo("%d physical devices found :\n", count);
	VkPhysicalDevice* physicalDevices = malloc(sizeof * physicalDevices * count);
	vkEnumeratePhysicalDevices(context->instance, &count, physicalDevices);

	static const VkPhysicalDeviceType priority[] =
	{
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
		VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
		VK_PHYSICAL_DEVICE_TYPE_CPU
	};

	for (uint32_t j = 0; j < count; j++)
	{
		vkGetPhysicalDeviceProperties(physicalDevices[j], &physicalDeviceProperties);
		logInfo("  %d : '%s' :\n", j, physicalDeviceProperties.deviceName);
	}

	for(uint32_t i = 0; i < sizeof priority / sizeof priority[0]; i++)
	{
		for (uint32_t j = 0; j < count; j++)
		{
			vkGetPhysicalDeviceProperties(physicalDevices[j], &physicalDeviceProperties);

			if (physicalDeviceProperties.deviceType == priority[i])
			{
				matchingPhysicalDevice = physicalDevices[j];
				context->physicalDeviceProperties = physicalDeviceProperties;
				break;
			}
		}

		if(matchingPhysicalDevice)
			break;
	}

	free(physicalDevices);

	if (matchingPhysicalDevice == NULL)
	{
		logError("Unable to find a suitable physical device");
		return -1;
	}

	context->physicalDevice = matchingPhysicalDevice;
	getPhysicalDeviceCapabilities(context);
	getMsaaCapabilities();

	return 0;
}


void getMsaaCapabilities(void)
{
	if(engine.settings.useMSAA)
	{
		VkSampleCountFlags framebufferSampleCount = engine.vulkanContext.physicalDeviceProperties.limits.framebufferColorSampleCounts;

		if(engine.settings.useDepthBuffer)
			framebufferSampleCount &= engine.vulkanContext.physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if(framebufferSampleCount & VK_SAMPLE_COUNT_64_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_64_BIT;
		else if(framebufferSampleCount & VK_SAMPLE_COUNT_32_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_32_BIT;
		else if(framebufferSampleCount & VK_SAMPLE_COUNT_16_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_16_BIT;
		else if(framebufferSampleCount & VK_SAMPLE_COUNT_8_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_8_BIT;
		else if(framebufferSampleCount & VK_SAMPLE_COUNT_4_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_4_BIT;
		else if(framebufferSampleCount & VK_SAMPLE_COUNT_2_BIT)
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_2_BIT;
		else
			engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_1_BIT;
	}
	else
		engine.vulkanContext.MSAASampling = VK_SAMPLE_COUNT_1_BIT;
}


void getPhysicalDeviceCapabilities(vulkanContext* context)
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, NULL);
	fprintf(stderr, "    %d queue family found:\n", queueFamilyCount);
	VkQueueFamilyProperties* queueFamilyProperties = malloc(sizeof * queueFamilyProperties * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, queueFamilyProperties);

	context->graphicsQueueFamilyIndex = UINT32_MAX;
	context->presentQueueFamilyIndex = UINT32_MAX;

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		int canPresentImages = glfwGetPhysicalDevicePresentationSupport(context->instance, context->physicalDevice, i);

		fprintf(stderr, "      queue family %d :\n", i);
		fprintf(stderr, "        queueFlags : %#010x.\n", queueFamilyProperties[i].queueFlags);
		fprintf(stderr, "        queueCount : %d.\n", queueFamilyProperties[i].queueCount);
		fprintf(stderr, "        can present images : %s.\n", canPresentImages ? "true" : "false");

		if (context->graphicsQueueFamilyIndex == UINT32_MAX &&
			queueFamilyProperties[i].queueCount >= 1 &&
			queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			context->graphicsQueueFamilyIndex = i;

		if (context->presentQueueFamilyIndex == UINT32_MAX &&
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

	// TODO : check this, many interesting stuff there
	//engine.vulkanContext.physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceInfo.pEnabledFeatures = &engine.vulkanContext.physicalDeviceFeatures;

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
		logError("Could not create device : missing requested extension(s)");
	else if( result == VK_ERROR_FEATURE_NOT_PRESENT)
		logError("Could not create device : missing requested feature(s)");
	else
		logError("Could not create device (reason : %d)", result);

	return result;
}


#define getInstanceProcAddr(vulkanContext, name)                                                                            \
	do {                                                                                                                    \
		if((vulkanContext->name = (PFN_vk##name)glfwGetInstanceProcAddress(vulkanContext->instance, "vk" #name)) == NULL)   \
		{                                                                                                                   \
			logWarning("Failed to get 'vk" #name " pointer from loader'");                                                  \
			res = -1;                                                                                                       \
		}                                                                                                                   \
	} while(0)                                                                                                              \


#define getDeviceProcAddr(vulkanContext, name)                                                                                  \
	do {                                                                                                                        \
		if((vulkanContext->name = (PFN_vk##name)vulkanContext->GetDeviceProcAddr(vulkanContext->device, "vk" #name)) == NULL)   \
		{                                                                                                                       \
			logWarning("Failed to get 'vk" #name " pointer from loader'");                                                      \
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
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
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
		logError("No suitable format found");
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
	uint32_t imageCount;

	if(context->surfaceCapabilities.maxImageCount == 0)
		imageCount = context->surfaceCapabilities.minImageCount + 1;
	else
		imageCount = clampu(
			context->surfaceCapabilities.minImageCount + 1,
			context->surfaceCapabilities.minImageCount,
			context->surfaceCapabilities.maxImageCount);

	if(context->surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		context->swapchainExtent = context->surfaceCapabilities.currentExtent;
	}
	else
	{
		VkExtent2D windowExtent = { (uint32_t)engine.framebufferWidth, (uint32_t)engine.framebufferHeight };

		context->swapchainExtent = clampExtent2D(
			windowExtent,
			context->surfaceCapabilities.minImageExtent,
			context->surfaceCapabilities.maxImageExtent);
	}

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
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// If the two queues are the same
	if(context->graphicsQueueFamilyIndex == context->presentQueueFamilyIndex)
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		// we need a bit more work as the image needs to be shared by two distinct queues
		uint32_t queueFamilyIndices[2] = {
			context->graphicsQueueFamilyIndex,
			context->presentQueueFamilyIndex
		};

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = sizeof queueFamilyIndices / sizeof *queueFamilyIndices;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	VkResult result = context->CreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapchain);

	if (result != VK_SUCCESS)
		logError("Swapchain creation failed");

	return result;
}


int recreateSwapChain(void)
{
	int result = -1;

	for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		engine.vulkanContext.needCommandBufferUpdate[i] = VK_TRUE;

	vkDeviceWaitIdle(engine.vulkanContext.device);
	cleanSwapchain(&engine.vulkanContext);

	// TODO : clean this ugly mess
	engine.vulkanContext.GetPhysicalDeviceSurfaceCapabilitiesKHR(engine.vulkanContext.physicalDevice, engine.vulkanContext.surface, &engine.vulkanContext.surfaceCapabilities);

	if(!createSwapchain(&engine.vulkanContext)
		&& !createImageViews(&engine.vulkanContext)
		&& !createColorResources()
		&& !createDepthBufferResources()
		&& !createFramebuffer(&engine.vulkanContext))
	{
		result = 0;
	}

	return result;
}


int createImageViews(vulkanContext* context)
{
	context->GetSwapchainImagesKHR(context->device, context->swapchain, &context->imageCount, NULL);
	context->images = malloc(sizeof *context->images * context->imageCount);
	context->imageViews = malloc(sizeof *context->imageViews * context->imageCount);
	context->GetSwapchainImagesKHR(context->device, context->swapchain, &context->imageCount, context->images);

	for (uint32_t i = 0; i < context->imageCount; i++)
		if(imageViewCreate2D(context->images[i], 1, context->surfaceFormat.format, &context->imageViews[i]))
			return -1;

	return 0;
}


int createRenderPass(vulkanContext* context)
{
	const uint32_t colorAttachmentCount = 2; // color attachment + color attachment resolve for MSAA
	uint32_t attachmentCount = colorAttachmentCount;

	if(engine.settings.useDepthBuffer)
		attachmentCount++;

	VkAttachmentDescription *attachments = malloc(sizeof *attachments * attachmentCount);

	// This color attachment is the one on which rendering happens with MSAA
	VkAttachmentDescription colorAttachment = {
		.format = context->surfaceFormat.format,
		.samples = engine.vulkanContext.MSAASampling,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // clear framebuffer between each rendering
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // write to the framebuffer when rendering
		// no stencil buffer so we don't care about it
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		// layout before and after rendering
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // don't care, we'll clear it anyway
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // layout suitable for present in swapchain
	};

	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0, // index of the attachment
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	// This attachment is for presenting only
	VkAttachmentDescription resolveAttachment = {
		.format = engine.vulkanContext.surfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // we'll overwrite it anyway
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		// no stencil buffer so we don't care about it
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference resolveAttachmentRef = {
		.attachment = 1, // index of the attachment
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	attachments[0] = colorAttachment;
	attachments[1] = resolveAttachment;

	// first and single subpass, containing one single color attachment reference, referencing our unique attachment
	VkSubpassDescription subpass = { 0 };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // graphics, no compute or RT
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = &resolveAttachmentRef;

	// Implicit dependency
	VkSubpassDependency dependency = { 0 };
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentReference depthAttachmentRef;

	if(engine.settings.useDepthBuffer)
	{
		VkAttachmentDescription depthAttachment = { 0 };
		depthAttachment.format = engine.vulkanContext.depthFormat;
		depthAttachment.samples = engine.vulkanContext.MSAASampling;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// no stencil buffer so we don't care about it
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[colorAttachmentCount] = depthAttachment;

		depthAttachmentRef.attachment = colorAttachmentCount;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	// list dependencies in case of multiple passes
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(context->device, &renderPassInfo, NULL, &context->renderPass);
	free(attachments);

	if (result)
		logError("Error while creating renderpass");

	return result;
}


int createFramebuffer(vulkanContext* context)
{
	context->swapChainFramebuffers = malloc(sizeof * context->swapChainFramebuffers * context->imageCount);

	VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = context->renderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.width = context->swapchainExtent.width;
	framebufferInfo.height = context->swapchainExtent.height;
	framebufferInfo.layers = 1;

	if(engine.settings.useDepthBuffer)
		framebufferInfo.attachmentCount++;

	for (uint32_t i = 0; i < context->imageCount; i++)
	{
		VkImageView attachments[] = { engine.vulkanContext.colorImageView, context->imageViews[i], engine.vulkanContext.depthImageView };

		framebufferInfo.pAttachments = attachments;
		VkResult result = vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &context->swapChainFramebuffers[i]);

		if (result != VK_SUCCESS)
		{
			logError("Failed to create framebuffer %d/%d", i + 1, context->imageCount);

			return result;
		}
	}

	return VK_SUCCESS;
}


int createCommandPool(vulkanContext* context)
{
	// main command pool
	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = context->graphicsQueueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result = vkCreateCommandPool(context->device, &poolInfo, NULL, &context->commandPool);

	if (result != VK_SUCCESS)
		logError("Error while creating the main command pool");

	// dedicated to buffer copies
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	result = vkCreateCommandPool(context->device, &poolInfo, NULL, &context->bufferCopyCommandPool);

	if (result != VK_SUCCESS)
		logError("Error while creating the transient command pool");

	return result;
}


int createColorResources(void)
{
	imageCreate2D(
		engine.vulkanContext.swapchainExtent.width,
		engine.vulkanContext.swapchainExtent.height,
		1,
		VK_FORMAT_B8G8R8A8_SRGB,
		engine.vulkanContext.MSAASampling,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		&engine.vulkanContext.colorImage,
		&engine.vulkanContext.colorImageMemory);

	imageViewCreate(
		engine.vulkanContext.colorImage,
		1,
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		&engine.vulkanContext.colorImageView);

	return VK_SUCCESS;
}


int createDepthBufferResources(void)
{
	if(engine.settings.useDepthBuffer)
	{
		engine.vulkanContext.depthFormat = VK_FORMAT_D32_SFLOAT;

		imageCreateDepth(
			engine.vulkanContext.swapchainExtent.width,
			engine.vulkanContext.swapchainExtent.height,
			engine.vulkanContext.depthFormat,
			engine.vulkanContext.MSAASampling,
			&engine.vulkanContext.depthImage,
			&engine.vulkanContext.depthImageMemory);

		imageViewCreateDepth(
			engine.vulkanContext.depthImage,
			engine.vulkanContext.depthFormat,
			&engine.vulkanContext.depthImageView);
	}

	return VK_SUCCESS;
}


int createCommandBuffers(void)
{
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = engine.vulkanContext.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	VkResult result = vkAllocateCommandBuffers(engine.vulkanContext.device, &allocInfo, engine.vulkanContext.commandBuffers);

	if (result != VK_SUCCESS)
		logError("Failed to allocate command buffers");

	return result;
}


int updateCommandBuffers(uint32_t cmdBufferIndex)
{
	VkResult result = VK_SUCCESS;

	if(engine.vulkanContext.needCommandBufferUpdate[cmdBufferIndex] == VK_FALSE)
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

	// viewport taking all the space available
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)engine.vulkanContext.swapchainExtent.width,
		.height = (float)engine.vulkanContext.swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	// no scissoring, we take the whole viewport
	VkRect2D scissor = {
		.offset.x = 0,
		.offset.y = 0,
		.extent = engine.vulkanContext.swapchainExtent
	};

	// clear color (see VK_ATTACHMENT_LOAD_OP_CLEAR)
	VkClearColorValue clearColor = {{ 0.03f, 0.03f, 0.03f, 1.0f }};
	VkClearDepthStencilValue clearDepth = { 1.f };
	VkClearValue clearValues[] = {{ .color = clearColor }, { .color = clearColor }, { .depthStencil = clearDepth }};
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues; // follow the same order as attachments

	if(engine.settings.useDepthBuffer)
		renderPassInfo.clearValueCount++;

	vkResetCommandBuffer(engine.vulkanContext.commandBuffers[cmdBufferIndex], 0);
	result = vkBeginCommandBuffer(engine.vulkanContext.commandBuffers[cmdBufferIndex], &beginInfo);

	if (result != VK_SUCCESS)
	{
		logError("Failed to begin recording command buffer %d/%d", cmdBufferIndex + 1, MAX_FRAMES_IN_FLIGHT);

		return result;
	}

	vkCmdSetViewport(engine.vulkanContext.commandBuffers[cmdBufferIndex], 0, 1, &viewport);
	vkCmdSetScissor(engine.vulkanContext.commandBuffers[cmdBufferIndex], 0, 1, &scissor);
	renderPassInfo.framebuffer = engine.vulkanContext.swapChainFramebuffers[cmdBufferIndex];

	vkCmdBeginRenderPass(engine.vulkanContext.commandBuffers[cmdBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // VK_SUBPASS_CONTENTS_INLINE for primary
	sceneDraw(engine.scene, engine.vulkanContext.commandBuffers[cmdBufferIndex]);
	vkCmdEndRenderPass(engine.vulkanContext.commandBuffers[cmdBufferIndex]);

	result = vkEndCommandBuffer(engine.vulkanContext.commandBuffers[cmdBufferIndex]);

	if (result != VK_SUCCESS)
	{
		logError("Failed to end recording command buffer %d/%d", cmdBufferIndex + 1, MAX_FRAMES_IN_FLIGHT);

		return result;
	}

	engine.vulkanContext.needCommandBufferUpdate[cmdBufferIndex] = VK_FALSE;

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
			logError("Failed to create sync objets");

			return -1;
		}
	}

	return 0;
}


void render(vulkanContext *context)
{
	uint32_t imageIndex;
	VkResult result = context->AcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->imageAvailableSemaphore[context->currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();

		return;
	}
	else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		logError("AcquireNextImageKHR() failed");

		return;
	}

	vkResetFences(context->device, 1, &context->inFlightFences[context->currentFrame]);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

	// one wait stage (color writing, and it's associated semaphore). Could be more stages here
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &context->imageAvailableSemaphore[context->currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;

	// command buffer associated with the image available
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &engine.vulkanContext.commandBuffers[context->currentFrame];

	// semaphore to signal when the command buffer's execution is finished
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &context->renderFinishedSemaphore[context->currentFrame];

	result = vkQueueSubmit(context->graphicQueue, 1, &submitInfo, context->inFlightFences[context->currentFrame]);

	if (result != VK_SUCCESS)
		logError("Failed to submit draw command buffer");

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &context->renderFinishedSemaphore[context->currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context->swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = &result; // Optional when only one swapchain, provide an array in case of multiple swapchains

	result = context->QueuePresentKHR(context->presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || engine.vulkanContext.framebufferResized)
	{
		engine.vulkanContext.framebufferResized = VK_FALSE;
		recreateSwapChain();
	}
	else if(result)
		logError("QueuePresentKHR() failed");
}


uint32_t clampu(uint32_t n, uint32_t min, uint32_t max)
{
	if (n < min)
		return min;

	if (n > max)
		return max;

	return n;
}


float clampf(float f, float min, float max)
{
	if (f < min)
		return min;

	if (f > max)
		return max;

	return f;
}


VkExtent2D clampExtent2D(VkExtent2D e, VkExtent2D min, VkExtent2D max)
{
	VkExtent2D res = {
		.height = clampu(e.height, min.height, max.height),
		.width = clampu(e.width, min.width, max.width)
	};

	return res;
}


void destroyVulkanContext(vulkanContext* context)
{
	if (context->device)
	{
		vkDeviceWaitIdle(context->device);
		cleanSwapchain(context);
		vkDestroyRenderPass(context->device, context->renderPass, NULL);
		vkDestroySampler(engine.vulkanContext.device, engine.vulkanContext.textureSamplers.linear, NULL);
		vkDestroySampler(engine.vulkanContext.device, engine.vulkanContext.textureSamplers.nearest, NULL);

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(context->device, context->imageAvailableSemaphore[i], NULL);
			vkDestroySemaphore(context->device, context->renderFinishedSemaphore[i], NULL);
			vkDestroyFence(context->device, context->inFlightFences[i], NULL);
		}

		vkFreeCommandBuffers(engine.vulkanContext.device, engine.vulkanContext.commandPool, MAX_FRAMES_IN_FLIGHT, engine.vulkanContext.commandBuffers);
		vkDestroyCommandPool(engine.vulkanContext.device, engine.vulkanContext.commandPool, NULL);
		vkDestroyCommandPool(engine.vulkanContext.device, engine.vulkanContext.bufferCopyCommandPool, NULL);
		vkDestroyDevice(context->device, NULL);
	}

	if (context->instance)
	{
		vkDestroySurfaceKHR(context->instance, context->surface, NULL);

		if(context->DestroyDebugUtilsMessengerEXT)
			context->DestroyDebugUtilsMessengerEXT(context->instance, context->messenger, NULL);

		vkDestroyInstance(context->instance, NULL);
	}

	memset(context, 0, sizeof(vulkanContext));
}


void cleanImageViews(void)
{
	for (uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
		vkDestroyImageView(engine.vulkanContext.device, engine.vulkanContext.imageViews[i], NULL);

	free(engine.vulkanContext.images);
	free(engine.vulkanContext.imageViews);
}


void cleanFrameBuffer(void)
{
	for (uint32_t i = 0; i < engine.vulkanContext.imageCount; i++)
		vkDestroyFramebuffer(engine.vulkanContext.device, engine.vulkanContext.swapChainFramebuffers[i], NULL);

	free(engine.vulkanContext.swapChainFramebuffers);
}


void cleanColorResources(void)
{
	vkDestroyImageView(engine.vulkanContext.device, engine.vulkanContext.colorImageView, NULL);
	vkDestroyImage(engine.vulkanContext.device, engine.vulkanContext.colorImage, NULL);
	vkFreeMemory(engine.vulkanContext.device, engine.vulkanContext.colorImageMemory, NULL);
}


void cleanDepthBufferResources(void)
{
	if(engine.settings.useDepthBuffer)
	{
		vkDestroyImageView(engine.vulkanContext.device, engine.vulkanContext.depthImageView, NULL);
		vkDestroyImage(engine.vulkanContext.device, engine.vulkanContext.depthImage, NULL);
		vkFreeMemory(engine.vulkanContext.device, engine.vulkanContext.depthImageMemory, NULL);
	}
}


void cleanSwapchain(vulkanContext* context)
{
	cleanFrameBuffer();
	cleanImageViews();
	cleanColorResources();
	cleanDepthBufferResources();
	context->DestroySwapchainKHR(context->device, context->swapchain, NULL);
}


scene denymGetScene(void)
{
	return engine.scene;
}


int createCaches(void)
{
	engine.caches.textureCache = resourceCacheCreate();
	engine.caches.shaderCache = resourceCacheCreate();

	VkPipelineCacheCreateInfo pipelineCacheInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

	if(vkCreatePipelineCache(engine.vulkanContext.device, &pipelineCacheInfo, NULL, &engine.caches.pipelineCache))
	{
		logError("Failed to create graphic pipeline cache");

		return -1;
	}

	return 0;
}


void destroyCaches(void)
{
	resourceCacheDestroy(engine.caches.textureCache);
	resourceCacheDestroy(engine.caches.shaderCache);
	vkDestroyPipelineCache(engine.vulkanContext.device, engine.caches.pipelineCache, NULL);
}


void updateMetrics(void)
{
	float now = getUptime();

	engine.metrics.time.lastRenderTime = now - engine.metrics.time.currentFrame;
	engine.metrics.time.lastFrame = engine.metrics.time.currentFrame;
	engine.metrics.frames.totalCount++;
	engine.metrics.frames.lastWindowCount++;

	if(now - engine.metrics.time.frameWindowStart > 1)
	{
		engine.metrics.frames.fps = (float)engine.metrics.frames.lastWindowCount / (now - engine.metrics.time.frameWindowStart);
		engine.metrics.frames.lastWindowCount = 0;
		engine.metrics.time.frameWindowStart = now;

		char title[128];
		snprintf(title, sizeof title, "%s - FPS: %.1f", APP_NAME, engine.metrics.frames.fps);
		glfwSetWindowTitle(engine.window, title);
	}
}


void updateCamera(void)
{
	if(engine.scene->camera)
	{
		float speed = engine.metrics.time.sinceLastFrame * 5;
		float speed_x = 0;
		float speed_y = 0;
		float speed_z = 0;

		if(glfwGetKey(engine.window, GLFW_KEY_W) == GLFW_PRESS)
			speed_x += speed;
		if(glfwGetKey(engine.window, GLFW_KEY_S) == GLFW_PRESS)
			speed_x -= speed;
		if(glfwGetKey(engine.window, GLFW_KEY_D) == GLFW_PRESS)
			speed_y += speed;
		if(glfwGetKey(engine.window, GLFW_KEY_A) == GLFW_PRESS)
			speed_y -= speed;
		if(glfwGetKey(engine.window, GLFW_KEY_SPACE) == GLFW_PRESS)
			speed_z += speed;
		if(glfwGetKey(engine.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			speed_z -= speed;

		cameraMove(engine.scene->camera, speed_x, speed_y, speed_z);

		float angularSpeed = engine.metrics.time.sinceLastFrame * 0.1f;
		float yaw = engine.input.mouse.cursor.diff.x * angularSpeed;
		float pitch = engine.input.mouse.cursor.diff.y * angularSpeed;

		if(engine.input.mouse.buttons.left)
			cameraRotate(engine.scene->camera, yaw, pitch, 0);

		if(glfwJoystickPresent(0))
		{
			if(engine.input.controller.triggers.zl)
				speed = engine.metrics.time.sinceLastFrame * 15;
			else
				speed = engine.metrics.time.sinceLastFrame * 5;

			if(engine.scene->camera->type == CAMERA_TYPE_PERSPECTIVE)
			{
				if(engine.input.controller.triggers.l)
					cameraSetFov(engine.scene->camera, 60);
				else
					cameraSetFov(engine.scene->camera, 90);
			}

			angularSpeed *= 25;

			speed_x = -engine.input.controller.leftStick.axis.y * speed;
			speed_y = engine.input.controller.leftStick.axis.x * speed;

			if(engine.input.controller.buttons.x)
				speed_z += speed;
			if(engine.input.controller.buttons.b)
				speed_z -= speed;

			yaw = engine.input.controller.rightStick.axis.x * angularSpeed;
			pitch = -engine.input.controller.rightStick.axis.y * angularSpeed;

			cameraMove(engine.scene->camera, speed_x, speed_y, speed_z);
			cameraRotate(engine.scene->camera, yaw, pitch, 0);
		}

		if(engine.input.mouse.scroll.y != 0)
		{
			if(engine.scene->camera->type == CAMERA_TYPE_PERSPECTIVE)
			{
				float fov = clampf(engine.scene->camera->fov - engine.input.mouse.scroll.y, 60, 90);

				cameraSetFov(engine.scene->camera, fov);
			}
			else if(engine.scene->camera->type == CAMERA_TYPE_ORTHOGRAPHIC)
			{
				float zoom = clampf(engine.scene->camera->zoom - engine.input.mouse.scroll.y, 1, 1000);

				cameraSetZoom(engine.scene->camera, zoom);
			}
		}
	}
}


int createTextureSamplers(void)
{
	return textureCreateSampler(&engine.vulkanContext.textureSamplers.linear, VK_FILTER_LINEAR) ||
		textureCreateSampler(&engine.vulkanContext.textureSamplers.nearest, VK_FILTER_NEAREST);
}
