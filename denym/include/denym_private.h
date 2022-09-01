#ifndef _denym_private_h_
#define _denym_private_h_


#include "denym.h"
#include "denym_common.h"

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
	VkFramebuffer *swapChainFramebuffers;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	VkCommandPool bufferCopyCommandPool;
	VkBool32 framebufferResized;

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


typedef struct geometry_t
{
	uint32_t vertexCount;

	// attributes
	uint32_t attribCount;
	float *positions;
	float *colors;
	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;

	// constants

	// uniforms
} geometry_t;


typedef struct renderable_t
{
	geometry_t geometry; // statically allocated for now
	const char *vertShaderName;
	const char *fragShaderName;
	VkCommandBuffer* commandBuffers; // draw cmds
	VkPipelineLayout pipelineLayout; // uniforms sent to shaders
	VkPipeline pipeline;             // type of render
} renderable_t;


typedef struct denym
{
	vulkanContext vulkanContext;
	GLFWwindow *window;

	renderable_t renderable; // statically allocated for now
} denym;


void glfwErrorCallback(int error, const char* description);

void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height);

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

int recreateSwapChain(void);

int createImageViews(vulkanContext* context);

int createRenderPass(vulkanContext* context);

int createPipeline(renderable renderable);

int createFramebuffer(vulkanContext* context);

int createCommandPool(vulkanContext* context);

int createVertexBuffers(geometry geometry);

int createBuffer(VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *vertexBufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags bufferUsage);

int createVertexBuffer(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src);

int createVertexBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src);

int copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

int findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index);

int createCommandBuffers(vulkanContext* context, renderable renderable);

int createSynchronizationObjects(vulkanContext* context);

void render(vulkanContext* context, renderable renderable);

uint32_t clamp(uint32_t n, uint32_t min, uint32_t max);

VkExtent2D clampExtent2D(VkExtent2D e, VkExtent2D min, VkExtent2D max);

void destroyVulkanContext(vulkanContext* context);

void cleanSwapchain(vulkanContext* context);

void cleanImageViews(void);

void cleanFrameBuffer(void);

void cleanCommandBuffers(void);


#endif
