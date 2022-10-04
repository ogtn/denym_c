#include "logger.h"
#include "denym_common.h"

#include <stdarg.h>

#ifdef __GNUC__
    #define LOG_INFO "\x1B[32mINFO\x1B[0m"
    #define LOG_WARN "\x1B[33mWARN\x1B[0m"
    #define LOG_ERR "\x1B[31mERROR\x1B[0m"
#else
    #define LOG_INFO "INFO"
    #define LOG_WARN "WARN"
    #define LOG_ERR "ERROR"
#endif


static void logMsg(const char *file, int line, const char *function, const char *level, const char *format, va_list list);


void logInfoFull(const char *file, int line, const char *function, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    logMsg(file, line, function, LOG_INFO, format, list);
    va_end(list);
}


void logErrorFull(const char *file, int line, const char *function, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    logMsg(file, line, function, LOG_ERR, format, list);
    va_end(list);
}


void logWarningFull(const char *file, int line, const char *function, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    logMsg(file, line, function, LOG_WARN, format, list);
    va_end(list);
}


void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "[%.6f][%s] GLFW error %d occured : '%s'", getUptime(), LOG_ERR, error, description);
}


VKAPI_ATTR VkBool32 VKAPI_CALL vulkanErrorCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
    const char *level = LOG_ERR;

	if(messageSeverity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
		level = LOG_INFO;
	else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		level = LOG_WARN;

    fprintf(stderr, "[%.6f][%s] %s\n", getUptime(), level, pCallbackData->pMessage);

	return VK_FALSE;
}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

static void logMsg(const char *file, int line, const char *function, const char *level, const char *format, va_list list)
{
    // TODO: only one print here, use temp buffer to avoid interleaved message with multiple threads
    fprintf(stderr, "[%.6f][%s][%s:%d] %s(): ", getUptime(), level, file, line, function);
    vfprintf(stderr, format, list);
    fprintf(stderr, "\n");
}

#pragma clang diagnostic pop
