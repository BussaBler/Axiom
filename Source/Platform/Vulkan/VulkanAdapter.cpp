#include "axpch.h"
#include "VulkanAdapter.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

namespace Axiom {
	std::unique_ptr<Device> VulkanAdapter::createDevice() {
		return std::make_unique<VulkanDevice>(*this);
	}

	std::vector<VulkanAdapter> VulkanAdapter::getAvailableAdapters(VulkanInstance& instance) {
		Vk::ResultValue<std::vector<Vk::PhysicalDevice>> physicalDevicesResult = instance.getHandle().enumeratePhysicalDevices();
		AX_CORE_ASSERT(physicalDevicesResult.result == Vk::Result::eSuccess, "Failed to enumerate physical devices for Vulkan instance");
		AX_CORE_ASSERT(!physicalDevicesResult.value.empty(), "No Vulkan suitable physical devices found");

		std::vector<VulkanAdapter> adapters;
		for (const auto& device : physicalDevicesResult.value) {
			adapters.emplace_back(instance, device);
		}

		return adapters;
	}

	std::vector<Vk::QueueFamilyProperties> VulkanAdapter::getQueueFamilyProperties() const {
		std::vector<Vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevice.getQueueFamilyProperties();
		return queueFamiliesProperties;
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

	uint32_t VulkanAdapter::findMemoryType(uint32_t typeFilter, Vk::MemoryPropertyFlags properties) const {
		auto memoryProperties = getMemoryProperties();
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		AX_CORE_LOG_ERROR("Failed to find suitable memory type for device: {}", std::string_view(getProperties().deviceName));
		return 0;
	}

	std::vector<Vk::SurfaceFormatKHR> VulkanAdapter::getSurfaceFormats(Vk::SurfaceKHR surface) const {
		Vk::ResultValue<std::vector<Vk::SurfaceFormatKHR>> surfaceFormatsResult = physicalDevice.getSurfaceFormatsKHR(surface);
		AX_CORE_ASSERT(surfaceFormatsResult.result == Vk::Result::eSuccess, "Failed to get surface formats for device: {}", std::string_view(getProperties().deviceName));

		return surfaceFormatsResult.value;
	}

	std::vector<Vk::PresentModeKHR> VulkanAdapter::getPresentModes(Vk::SurfaceKHR surface) const {
		Vk::ResultValue<std::vector<Vk::PresentModeKHR>> presentModesResult = physicalDevice.getSurfacePresentModesKHR(surface);
		AX_CORE_ASSERT(presentModesResult.result == Vk::Result::eSuccess, "Failed to get present modes for device: {}", std::string_view(getProperties().deviceName));

		return presentModesResult.value;
	}

	Vk::SurfaceCapabilitiesKHR VulkanAdapter::getSurfaceCapabilities(Vk::SurfaceKHR surface) const {
		Vk::ResultValue<Vk::SurfaceCapabilitiesKHR> surfaceCapabilitiesResult = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		AX_CORE_ASSERT(surfaceCapabilitiesResult.result == Vk::Result::eSuccess, "Failed to get surface capabilities for device: {}", std::string_view(getProperties().deviceName));

		return surfaceCapabilitiesResult.value;
	}

	Vk::Format VulkanAdapter::getDepthFormat() const {
		std::array<Vk::Format,3> depthFormats = {
			Vk::Format::eD32SfloatS8Uint,
			Vk::Format::eD32Sfloat,
			Vk::Format::eD24UnormS8Uint
		};

		for (const auto& format : depthFormats) {
			Vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
			if (formatProperties.optimalTilingFeatures & Vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
				return format;
			}
			if (formatProperties.linearTilingFeatures & Vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
				return format;
			}
		}

		return Vk::Format::eUndefined;
	}

	bool VulkanAdapter::checkAvailableDeviceExtensions() const {
		Vk::ResultValue<std::vector<Vk::ExtensionProperties>> availableExtensionsResult = physicalDevice.enumerateDeviceExtensionProperties();
		AX_CORE_ASSERT(availableExtensionsResult.result == Vk::Result::eSuccess, "Failed to enumerate device extensions for device: {}", std::string_view(getProperties().deviceName));
		const auto& availableExtensions = availableExtensionsResult.value;
		
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
				AX_CORE_LOG_ERROR("Required Vulkan Device Extension not found for device: {} {}", required, std::string_view(getProperties().deviceName));
				return false;
			}
		}
		return true;
	}

	bool VulkanAdapter::checkSurfaceSupport(Vk::SurfaceKHR surface, uint32_t queueFamilyIndex) const {
		Vk::ResultValue<uint32_t> supported = physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface);
		return supported.result == Vk::Result::eSuccess && supported.value == VK_TRUE;
	}
}


