#ifndef _logger_h_
#define _logger_h_


#include "denym_common.h"


#ifdef __GNUC__
    #define CHK_FMT_AND_ARGS(fmt_pos) __attribute((format(printf, (fmt_pos), (fmt_pos + 1))))
    #define CHK_FMT_ONLY(fmt_pos) __attribute((format(printf, (fmt_pos), 0)))
#else
    #define CHK_FMT_AND_ARGS(fmt_pos)
    #define CHK_FMT_ONLY(fmt_pos)
#endif


#define logError(...) logErrorFull(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logWarning(...) logWarningFull(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define logInfo(...) logInfoFull(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


void logWarningFull(const char *file, int line, const char *function, const char *format, ...) CHK_FMT_AND_ARGS(4);

void logInfoFull(const char *file, int line, const char *function, const char *format, ...) CHK_FMT_AND_ARGS(4);

void logErrorFull(const char *file, int line, const char *function, const char *format, ...) CHK_FMT_AND_ARGS(4);

void glfwErrorCallback(int error, const char* description);

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanErrorCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

#endif
