#pragma once
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "Renderer/Core/Buffer.h"

namespace Axiom {
	class VulkanBuffer : public Buffer {
	public:
		VulkanBuffer(VulkanDevice& device, VkBufferUsageFlags usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind);
		~VulkanBuffer();
		void resize(uint64_t newSize, CommandPool& commandPool, Fence* fence, Queue& queue) override;
		void bind(uint64_t offset) override;
		void lock(uint64_t size, uint32_t flags, uint64_t offset = 0) override;
		void unlock() override;
		void copyFrom(const void* data, uint64_t size, uint32_t flags, uint64_t offset = 0) override;
		void copyTo(Buffer& destination, CommandPool& commandPool, Fence* fence, Queue& queue, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0) override;

	private:
		VkDeviceMemory memory;
		VulkanDevice& device;
	};
}