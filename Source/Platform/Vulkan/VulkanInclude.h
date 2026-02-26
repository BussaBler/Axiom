#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NAMESPACE Vk
#define VULKAN_HPP_NO_EXCEPTIONS

#if defined(AX_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(AX_PLATFORM_LINUX)
	#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.hpp>
