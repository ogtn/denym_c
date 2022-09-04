#ifndef _core_h
#define _core_h


#include "denym_common.h"


#define MAX_FRAMES_IN_FLIGHT 2

#define DECL_VK_PFN(name) PFN_vk##name name


typedef struct vulkanContext
{
	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	//VkPhysicalDeviceFeatures physicalDeviceFeatures;
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

	VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    uint64_t currentFrame; // 64bits for padding purpose only

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
} denym;


#endif
