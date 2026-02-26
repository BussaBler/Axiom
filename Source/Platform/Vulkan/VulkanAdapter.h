#pragma once
#include "Renderer/Adapter.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanInstance;

	struct QueueFamily {
		uint32_t maxQueues;
		uint32_t index;
		uint32_t availableIndex;
		Vk::QueueFlags flags;
	};

	class VulkanAdapter : public Adapter {
	public:
		VulkanAdapter(VulkanInstance& vkInstance, Vk::PhysicalDevice physicalDevice) : instance(vkInstance), physicalDevice(physicalDevice) {}
		~VulkanAdapter() = default;
		VulkanAdapter(const VulkanAdapter&) = default;
		std::unique_ptr<Device> createDevice() override;

		static std::vector<VulkanAdapter> getAvailableAdapters(VulkanInstance& instance);
		const Vk::PhysicalDeviceProperties& getProperties() const {
			static Vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
			return properties;
		}
		const Vk::PhysicalDeviceFeatures& getFeatures() const {
			static Vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();
			return features;
		}
		const Vk::PhysicalDeviceMemoryProperties& getMemoryProperties() const {
			static Vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
			return memoryProperties;
		}
		std::vector<Vk::QueueFamilyProperties> getQueueFamilyProperties() const;
		std::vector<QueueFamily> findQueueFamilies() const;
		uint32_t findMemoryType(uint32_t typeFilter, Vk::MemoryPropertyFlags properties) const;
		std::vector<Vk::SurfaceFormatKHR> getSurfaceFormats(Vk::SurfaceKHR surface) const;
		std::vector<Vk::PresentModeKHR> getPresentModes(Vk::SurfaceKHR surface) const;
		Vk::SurfaceCapabilitiesKHR getSurfaceCapabilities(Vk::SurfaceKHR surface) const;
		Vk::Format getDepthFormat() const;
		Vk::PhysicalDevice getHandle() const { return physicalDevice; }
		VulkanInstance& getInstance() const { return instance; }

		bool checkAvailableDeviceExtensions() const;
		bool checkSurfaceSupport(Vk::SurfaceKHR surface, uint32_t queueFamilyIndex) const;

	private:
		VulkanInstance& instance;
		Vk::PhysicalDevice physicalDevice;
	};
}

