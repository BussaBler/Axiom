#pragma once
#include "VulkanResource.h"

namespace Axiom {

	class VulkanBuffer : public VulkanResource {
	public:
		VulkanBuffer(VulkanDevice& vkDevice) : VulkanResource(vkDevice) {}
		~VulkanBuffer() override;

		void init(ResourceCreateInfo& createInfo);
		void lockMemory(void** mappedMemory, uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE, uint32_t flags = 0);
		void unlockMemory();
		void loadData(void* data, uint64_t size, uint64_t offset = 0, uint32_t flags = 0) override;
		void copyToBuffer(VulkanBuffer& destination, VkCommandPool commandPool, VkFence fence, VkQueue queue, uint64_t srcOffset = 0, uint64_t dstOffset = 0, uint64_t size = VK_WHOLE_SIZE);
		void bind(uint64_t offset = 0);

		VkBuffer getHandle() const { return buffer; }

	private:
		static VkBufferUsageFlags getBufferUsage(uint32_t usage);

	private:
		VkBuffer buffer = VK_NULL_HANDLE;
		bool locked = false;
	};
}

