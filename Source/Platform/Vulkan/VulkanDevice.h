#pragma once
#include "Renderer/GraphicsDevice.h"
#include <vulkan/vulkan.h>
#ifdef AX_PLATFORM_WINDOWS
#include "Platform/Windows/Win32Window.h"
#include <vulkan/vulkan_win32.h>
#endif // AX_PLATFORM_WINDOWS

#include <vector>
#include <string>

namespace Axiom {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		uint32_t graphicsFamily;
		uint32_t presentFamily;
		uint32_t computeFamily;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool computeFamilyHasValue = false;
		bool isComplete() const {
			return graphicsFamilyHasValue && presentFamilyHasValue && computeFamilyHasValue;
		}
	};

	class VulkanDevice : public GraphicsDevice {
	public:
		VulkanDevice(Window* window);
		~VulkanDevice();
		void submitCommandBuffer() override;

		VkCommandPool getCommandPool() const { return commandPool; }
		VkDevice getDevice() const { return device; }
		VkSurfaceKHR getSurface() const { return surface; }
		VkQueue getGraphicsQueue() const { return graphicsQueue; }
		VkQueue getPresentQueue() const { return presentQueue; }
		VkQueue getComputeQueue() const { return computeQueue; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;


	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		bool isDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions() const;
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void hasRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

	private:
#if AX_DEBUG
		const bool enableValidationLayers = true;
#else 
		const bool enableValidationLayers = false;
#endif // AX_DEBUG
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkCommandPool commandPool;
		VkDevice device;
		VkSurfaceKHR surface;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}

