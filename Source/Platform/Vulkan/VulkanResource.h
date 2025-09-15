#pragma once
#include "Renderer/Resource.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanView;

	class VulkanResource : public Resource {
	public:
		VulkanResource(VulkanDevice& vkDevice);
		virtual ~VulkanResource() override;

		uint32_t generateId() override;

		static VkFormat getVkFormat(ResourceFormat format);
		static ResourceFormat getResourceFormat(VkFormat format);

	protected:
		VulkanDevice& device;
		VkDeviceMemory memory;
		VkAccessFlags currentAccessFlags;
		VkDeviceSize memorySize;
		uint32_t id;
		std::unique_ptr<VulkanView> resourceView;

		static uint32_t idCounter;
		static std::mutex idMutex;
	};
}

