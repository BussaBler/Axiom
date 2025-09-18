#include "axpch.h"
#include "VulkanAdapter.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

namespace Axiom {
	std::unique_ptr<Device> VulkanAdapter::createDevice() {
		return std::make_unique<VulkanDevice>(*this);
	}

	std::vector<VulkanAdapter> VulkanAdapter::getAvailableAdapters(VulkanInstance& instance) {
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(instance.getHandle(), &deviceCount, nullptr);
		AX_CORE_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(instance.getHandle(), &deviceCount, physicalDevices.data());

		std::vector<VulkanAdapter> adapters;
		for (const auto& device : physicalDevices) {
			adapters.emplace_back(instance, device);
		}

		return adapters;
	}

	std::vector<VkQueueFamilyProperties> VulkanAdapter::getQueueFamilyProperties() const {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
		return queueFamilies;
	}

	std::vector<QueueFamily> VulkanAdapter::findQueueFamilies() const {
		auto queueFamilyProperties = getQueueFamilyProperties();
		std::vector<QueueFamily> queueFamilies;

		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			QueueFamily queueFamily{};
			queueFamily.index = i;
			queueFamily.flags = queueFamilyProperties[i].queueFlags;
			queueFamily.maxQueues = queueFamilyProperties[i].queueCount;
			queueFamily.availableIndex = 0;
			queueFamilies.push_back(queueFamily);
		}

		return queueFamilies;
	}

	uint32_t VulkanAdapter::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
		auto memoryProperties = getMemoryProperties();
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		AX_CORE_LOG_ERROR("Failed to find suitable memory type for device: {}", getProperties().deviceName);
		return 0;
	}

	std::vector<VkSurfaceFormatKHR> VulkanAdapter::getSurfaceFormats(VkSurfaceKHR surface) const {
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
		return formats;
	}

	std::vector<VkPresentModeKHR> VulkanAdapter::getPresentModes(VkSurfaceKHR surface) const {
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
		return presentModes;
	}

	VkSurfaceCapabilitiesKHR VulkanAdapter::getSurfaceCapabilities(VkSurfaceKHR surface) const {
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
		return capabilities;
	}

	VkFormat VulkanAdapter::getDepthFormat() const {
		std::array<VkFormat,3> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		for (const auto& format : depthFormats) {
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				return format;
			}
			if (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				return format;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}

	bool VulkanAdapter::checkAvailableDeviceExtensions() const {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
		std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		for (const char* required : requiredExtensions) {
			bool found = false;
			for (const auto& extension : availableExtensions) {
				if (strcmp(required, extension.extensionName) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				AX_CORE_LOG_ERROR("Required Vulkan Device Extension not found for device: {} {}", required, getProperties().deviceName);
				return false;
			}
		}
		return true;
	}

	bool VulkanAdapter::checkSurfaceSupport(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const {
		VkBool32 supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &supported);
		return supported == VK_TRUE;
	}
}


