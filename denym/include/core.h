#ifndef _core_h
#define _core_h


#include "denym_common.h"
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
	VkBool32 usDepthBuffer;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkFormat depthFormat;

	VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

	// command buffers
	VkBool32 needCommandBufferUpdate;
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

	// not sure if this deserves to be there...
	VkBool32 needRecreatePipeline;
	uint32_t __padding;

	// texture samplers
	VkSampler textureSampler; // TODO: find a better place

	// instance extension functions
	DECL_VK_PFN(GetDeviceProcAddr);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceSupportKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfaceFormatsKHR);
	DECL_VK_PFN(GetPhysicalDeviceSurfacePresentModesKHR);
	DECL_VK_PFN(CreateDebugUtilsMessengerEXT);
	DECL_VK_PFN(DestroyDebugUtilsMessengerEXT);

	// device extensions functions
	DECL_VK_PFN(DestroySurfaceKHR);
	DECL_VK_PFN(CreateSwapchainKHR);
	DECL_VK_PFN(DestroySwapchainKHR);
	DECL_VK_PFN(GetSwapchainImagesKHR);
	DECL_VK_PFN(AcquireNextImageKHR);
	DECL_VK_PFN(QueuePresentKHR);
	DECL_VK_PFN(SetDebugUtilsObjectNameEXT);
} vulkanContext;


typedef struct denym
{
	vulkanContext vulkanContext;

	GLFWwindow *window;
	int windowWidth;
	int windowHeight;
	int windowPosX;
	int windowPosY;
	VkBool32 isFullScreen;

	// TODO: add moar metrics : fps, render time etc.
	struct timespec uptime;
	uint64_t frameCount;
} denym;


#endif
