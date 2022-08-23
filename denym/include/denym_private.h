#ifndef _denym_private_h_
#define _denym_private_h_

#include "denym.h"


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


static const char* APP_NAME = "Denym WIP";
static const int APP_VERSION = VK_MAKE_API_VERSION(0, 0, 1, 0);

static const char* ENGINE_NAME = "Denym";
static const int ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 1, 0);

#define MAX_FRAMES_IN_FLIGHT 2

#define DECL_VK_PFN(name) PFN_vk##name name


typedef struct vulkanContext
{
	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	int graphicsQueueFamilyIndex;
	int presentQueueFamilyIndex;

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
	VkFramebuffer *swapChainFramebuffers;
	VkRenderPass renderPass;
	VkCommandPool commandPool;

	// geometry
	VkCommandBuffer* commandBuffers; // draw cmds
	VkPipelineLayout pipelineLayout; // uniforms sent to shaders
	VkPipeline pipeline;             // type of render

	VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    int currentFrame;

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

/*
typedef struct dnm_object
{
	const char *vertex_shader;
	const char *fragment_shader;

	int nb_vertices;

	// vertex attributes
	float *positions;
	float *colors;

	//int nb_uniforms;
	//dnm_uniform uniforms;

	// int nb_constant;
	// dnm_shader_constants;
} dnm_object;


int dnm_init_object(
	const char *vertex_shader,
	const char *fragment_shader,
	int nb_vertices,
	dnm_object *object)
{
	object->positions = NULL;
	object->colors = NULL;
	object->nb_vertices = nb_vertices;

	return 0;
}
*/


void glfwErrorCallback(int error, const char* description);

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanErrorCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

GLFWwindow* createWindow(int width, int height);

int getPhysicalDevice(vulkanContext* context);

void getPhysicalDeviceCapabilities(vulkanContext* context);

int getDevice(vulkanContext* context);

int createVulkanInstance(vulkanContext* context);

int getInstanceExtensionsAddr(vulkanContext* context);

int getDeviceExtensionsAddr(vulkanContext* context);

int getSwapchainCapabilities(vulkanContext* context);

int createSwapchain(vulkanContext* context);

int createImageViews(vulkanContext* context);

int loadShader(vulkanContext* context, const char* name, VkShaderModule *outShaderr);

int createRenderPass(vulkanContext* context);

int createPipeline(vulkanContext* context, const char *vertShaderName, const char *fragShaderName);

int createFramebuffer(vulkanContext* context);

int createCommandPool(vulkanContext* context);

int createCommandBuffers(vulkanContext* context, uint32_t vertexCount);

int createSynchronizationObjects(vulkanContext* context);

void render(vulkanContext* context);

uint32_t clamp(uint32_t n, uint32_t min, uint32_t max);

VkExtent2D clampExtent2D(VkExtent2D e, VkExtent2D min, VkExtent2D max);

void destroyVulkanContext(vulkanContext* context);

void cleanSwapchain(vulkanContext* context);


#endif
