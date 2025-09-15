#pragma once
#include "Renderer/Adapter.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanInstance;

	struct QueueFamily {
		uint32_t maxQueues;
		uint32_t index;
		uint32_t availableIndex;
		VkQueueFlags flags;
	};

	class VulkanAdapter : public Adapter {
	public:
		VulkanAdapter(VulkanInstance& vkInstance, VkPhysicalDevice physicalDevice) : instance(vkInstance), physicalDevice(physicalDevice) {}
		~VulkanAdapter() = default;
		VulkanAdapter(const VulkanAdapter&) = default;
		std::unique_ptr<Device> createDevice() override;

		static std::vector<VulkanAdapter> getAvailableAdapters(VulkanInstance& instance);
		const VkPhysicalDeviceProperties& getProperties() const {
			static VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);
			return properties;
		}
		const VkPhysicalDeviceFeatures& getFeatures() const {
			static VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(physicalDevice, &features);
			return features;
		}
		const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const {
			static VkPhysicalDeviceMemoryProperties memoryProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
			return memoryProperties;
		}
		std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() const;
		std::vector<QueueFamily> findQueueFamilies() const;
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface) const;
		std::vector<VkPresentModeKHR> getPresentModes(VkSurfaceKHR surface) const;
		VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;
		VkFormat getDepthFormat() const;
		VkPhysicalDevice getHandle() const { return physicalDevice; }
		VulkanInstance& getInstance() const { return instance; }

		bool checkAvailableDeviceExtensions() const;
		bool checkSurfaceSupport(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;

	private:
		VulkanInstance& instance;
		VkPhysicalDevice physicalDevice;
	};
}

