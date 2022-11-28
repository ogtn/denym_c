#ifndef _denym_private_h_
#define _denym_private_h_


#include "denym_common.h"
#include "core.h"


static const char* APP_NAME = "Denym WIP";
static const int APP_VERSION = VK_MAKE_API_VERSION(0, 0, 1, 0);

static const char* ENGINE_NAME = "Denym";
static const int ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 1, 0);


void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height);

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

GLFWwindow* createWindow(int width, int height);

int getPhysicalDevice(vulkanContext* context);

void getMsaaCapabilities(void);

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

int createFramebuffer(vulkanContext* context);

int createCommandPool(vulkanContext* context);

int createColorResources(void);

int createDepthBufferResources(void);

int createCommandBuffers(void);

int updateCommandBuffers(uint32_t cmdBufferIndex);

int createSynchronizationObjects(vulkanContext* context);

void render(vulkanContext *context);

uint32_t clampu(uint32_t n, uint32_t min, uint32_t max);

float clampf(float f, float min, float max);

VkExtent2D clampExtent2D(VkExtent2D e, VkExtent2D min, VkExtent2D max);

void destroyVulkanContext(vulkanContext* context);

void cleanSwapchain(vulkanContext* context);

void cleanImageViews(void);

void cleanColorResources(void);

void cleanDepthBufferResources(void);

void cleanFrameBuffer(void);

scene denymGetScene(void);

int createCaches(void);

void destroyCaches(void);

void updateMetrics(void);

int createTextureSamplers(void);


#endif
