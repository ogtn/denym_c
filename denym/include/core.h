#ifndef _core_h
#define _core_h


#include "denym_common.h"
#include "resource_cache.h"
#include "input.h"

#include <time.h>


#define DECL_VK_PFN(name) PFN_vk##name name


typedef struct vulkanContext
{
	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;

	// MSAA
	VkSampleCountFlagBits MSAASampling;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkDevice device;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkQueue graphicQueue;
	VkQueue presentQueue;
	VkDebugUtilsMessengerEXT messenger;

	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	VkImage* images;
	VkImageView* imageViews;
	uint32_t imageCount;
	VkBool32 framebufferResized;
	VkFramebuffer *swapChainFramebuffers;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	VkCommandPool bufferCopyCommandPool;

	// depth buffer
	VkFormat depthFormat;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

	// command buffers
	VkBool32 needCommandBufferUpdate[MAX_FRAMES_IN_FLIGHT];
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

	// TODO: find a better place for this
	// texture samplers
	struct
	{
		VkSampler linear;
		VkSampler nearest;
	} textureSamplers;


	// instance extension functions
	DECL_VK_PFN(GetDeviceProcAddr);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceSupportKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceFormatsKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfacePresentModesKHR);
	DECL_VK_PFN(CreateDebugUtilsMessengerEXT);
	DECL_VK_PFN(DestroyDebugUtilsMessengerEXT);

	// device extensions functions
	DECL_VK_PFN(CreateSwapchainKHR);
	DECL_VK_PFN(DestroySwapchainKHR);
	DECL_VK_PFN(GetSwapchainImagesKHR);
	DECL_VK_PFN(AcquireNextImageKHR);
	DECL_VK_PFN(QueuePresentKHR);
	DECL_VK_PFN(SetDebugUtilsObjectNameEXT);
} vulkanContext;


typedef struct resourceCaches
{
	resourceCache textureCache;
} resourceCaches;


typedef struct denym
{
	vulkanContext vulkanContext;

	GLFWwindow *window;
	int windowWidth;
	int windowHeight;
	int windowPosX;
	int windowPosY;
	int framebufferWidth;
	int framebufferHeight;
	VkBool32 isFullScreen;
	VkBool32 shouldStop;
	input_t input;

	struct
	{
		struct
		{
			struct timespec startTime;
			float lastFrame;
			float sinceLastFrame;
			float currentFrame;
			float lastRenderTime;
			float frameWindowStart;
		} time;

		struct
		{
			uint64_t totalCount;
			uint32_t lastWindowCount;
			float fps;
		} frames;

	} metrics;

	texture textureFallback;
	scene scene;

	struct
	{
		resourceCache textureCache;
		resourceCache shaderCache;
		VkPipelineCache pipelineCache;
	} caches;

	struct
	{
		VkBool32 useMSAA;
		VkBool32 useDepthBuffer;
		VkBool32 cacheUniformMemory;
		VkBool32 cacheStorageBufferMemory;
		VkBool32 captureMouse;
	} settings;
} denym;


#endif
