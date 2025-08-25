#pragma once
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	class VulkanBuffer {
	public:
		VulkanBuffer(VulkanDevice& device, VkBufferUsageFlags usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind);
		~VulkanBuffer();
		void resize(uint64_t newSize, VkCommandPool commandPool, VkFence fence, VkQueue queue);
		void bind(uint64_t offset);
		void lock(uint64_t size, uint32_t flags, uint64_t offset = 0);
		void unlock();
		void copyFrom(const void* data, uint64_t size, uint32_t flags, uint64_t offset = 0);
		void copyTo(VkBuffer destination, VkCommandPool commandPool, VkFence fence, VkQueue queue, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
		VkBuffer getHandle() const { return handle; }
		uint64_t getSize() const { return totalSize; }
		VkBufferUsageFlags getUsage() const { return usage; }

	private:
		VkBuffer handle;
		VkDeviceMemory memory;
		VkBufferUsageFlags usage;
		VulkanDevice& device;
		uint64_t totalSize;
		bool isLocked = false;
		void* data = nullptr;
		int memoryIndex;
		uint32_t memoryPropertyFlags;
	};
}

