#pragma once
#include "Core/Assert.h"
#include "Renderer/Instance.h"
#include "VulkanInclude.h"

#if defined(AX_PLATFORM_WINDOWS)
	#include "Platform/Windows/Win32Window.h"
#elif defined(AX_PLATFORM_LINUX)
	#include "Platform/Linux/XLibWindow.h"
#endif

namespace Axiom {
	class VulkanInstance : public Instance {
	public:
		VulkanInstance() : Instance(GraphicsAPI::VULKAN), vkGetInstanceProcAddr(nullptr), instance(nullptr), debugMessenger(nullptr) {}
		~VulkanInstance();

		void init(const InstaceInfo& info) override;
		std::unique_ptr<Adapter> getAdapter() override;

		Vk::Instance getHandle() const { return instance; }

		Vk::SurfaceKHR createSurface(void* windowHandle) const;
		void destroySurface(Vk::SurfaceKHR surface) const;

	private:
		bool linkVulkan();
		std::vector<const char*> getRequiredExtensions() const;
		std::vector<const char*> getValidationLayers() const;

	private:
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

		Vk::Instance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}
