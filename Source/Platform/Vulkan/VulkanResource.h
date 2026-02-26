#pragma once
#include "Renderer/Resource.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;
	class VulkanView;

	class VulkanResource : public Resource {
	public:
		VulkanResource(VulkanDevice& vkDevice);
		virtual ~VulkanResource() override;

		uint32_t generateId() override;

		static Vk::Format getVkFormat(ResourceFormat format);
		static ResourceFormat getResourceFormat(Vk::Format format);
		static Vk::MemoryPropertyFlags getMemoryPropertyFlags(uint32_t memoryUsage);

	protected:
		VulkanDevice& device;
		Vk::DeviceMemory memory;
		Vk::AccessFlags currentAccessFlags;
		Vk::DeviceSize memorySize;
		uint32_t id;
		std::unique_ptr<VulkanView> resourceView;

	private:
		static uint32_t idCounter;
		static std::mutex idMutex;
	};
}

