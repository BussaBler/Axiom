#pragma once
#include "Core/Assert.h"
#include "Renderer/Instance.h"
#include <vulkan/vulkan.h>
#if defined(AX_PLATFORM_WINDOWS)
#include "Platform/Windows/Win32Window.h"
#include <vulkan/vulkan_win32.h>
#elif defined(AX_PLATFORM_LINUX)
#include "Platform/Linux/XLibWindow.h"
#include <vulkan/vulkan_xlib.h>
#endif

namespace Axiom {
	class VulkanInstance : public Instance {
	public:
		VulkanInstance() : Instance(GraphicsAPI::VULKAN), instance(VK_NULL_HANDLE), debugMessenger(VK_NULL_HANDLE) {}
		~VulkanInstance();

		void init(const InstaceInfo& info) override;
		std::unique_ptr<Adapter> getAdapter() override;

		VkInstance getHandle() const { return instance; }

		VkSurfaceKHR createSurface(void* windowHandle) const;
		void destroySurface(VkSurfaceKHR surface) const;

	private:
		std::vector<const char*> getRequiredExtensions() const;
		std::vector<const char*> getValidationLayers() const;

	private:
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}
