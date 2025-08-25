#include "axpch.h"
#include "VulkanBuffer.h"

namespace Axiom {
	VulkanBuffer::VulkanBuffer(VulkanDevice& vkDevice, VkBufferUsageFlags usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind)
		: device(vkDevice), usage(usage), totalSize(size), memoryPropertyFlags(memoryPropertyFlags) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		AX_CORE_ASSERT(vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr, &handle) == VK_SUCCESS, "Failed to create Vulkan buffer!");
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.getHandle(), handle, &memRequirements);
		memoryIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryIndex;
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle(), &allocInfo, nullptr, &memory) == VK_SUCCESS, "Failed to allocate Vulkan buffer memory!");
		
		if (shouldBind) {
			bind(0);
		}
	}

	VulkanBuffer::~VulkanBuffer() {
		if (isLocked) {
			unlock();
		}
		vkDestroyBuffer(device.getHandle(), handle, nullptr);
		vkFreeMemory(device.getHandle(), memory, nullptr);
	}

	void VulkanBuffer::resize(uint64_t newSize, VkCommandPool commandPool, VkFence fence, VkQueue queue) {
		if (isLocked) {
			unlock();
		}
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = newSize;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer newHandle;
		AX_CORE_ASSERT(vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr, &newHandle) == VK_SUCCESS, "Failed to create Vulkan buffer!");
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.getHandle(), newHandle, &memRequirements);
		memoryIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryIndex;
		VkDeviceMemory newMemory;
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle(), &allocInfo, nullptr, &newMemory) == VK_SUCCESS, "Failed to allocate Vulkan buffer memory!");
		
		vkBindBufferMemory(device.getHandle(), newHandle, newMemory, 0);
		copyTo(newHandle, commandPool, fence, queue, (std::min)(newSize, totalSize), 0, 0);

		vkDeviceWaitIdle(device.getHandle());
		vkDestroyBuffer(device.getHandle(), handle, nullptr);
		vkFreeMemory(device.getHandle(), memory, nullptr);
		handle = newHandle;
		totalSize = newSize;
		memory = newMemory;
	}

	void VulkanBuffer::bind(uint64_t offset) {
		AX_CORE_ASSERT(vkBindBufferMemory(device.getHandle(), handle, memory, offset) == VK_SUCCESS, "Failed to bind Vulkan buffer memory!");
	}

	void VulkanBuffer::lock(uint64_t size, uint32_t flags, uint64_t offset) {
		AX_CORE_ASSERT(!isLocked, "Vulkan buffer is already locked!");
		AX_CORE_ASSERT(vkMapMemory(device.getHandle(), memory, offset, size, flags, &data) == VK_SUCCESS, "Failed to map Vulkan buffer memory!");
		isLocked = true;
	}

	void VulkanBuffer::unlock() {
		AX_CORE_ASSERT(isLocked, "Vulkan buffer is not locked!");
		vkUnmapMemory(device.getHandle(), memory);
		data = nullptr;
		isLocked = false;
	}

	void VulkanBuffer::copyFrom(const void* srcData, uint64_t size, uint32_t flags, uint64_t offset) {
		void* localData;
		AX_CORE_ASSERT(vkMapMemory(device.getHandle(), memory, offset, size, flags, &localData) == VK_SUCCESS, "Failed to map Vulkan buffer memory!");
		memcpy(localData, srcData, size);
		vkUnmapMemory(device.getHandle(), memory);
	}

	void VulkanBuffer::copyTo(VkBuffer destination, VkCommandPool commandPool, VkFence fence, VkQueue queue, uint64_t size, uint64_t srcOffset, uint64_t dstOffset) {
		vkQueueWaitIdle(queue);
		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocate(commandPool);
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer.getHandle(), handle, destination, 1, &copyRegion);
		
		commandBuffer.end();
		VkCommandBuffer commandBufferHandle = commandBuffer.getHandle();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBufferHandle;
		
		AX_CORE_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence) == VK_SUCCESS, "Failed to submit Vulkan buffer copy command buffer!");
		if (fence) {
			vkWaitForFences(device.getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
			vkResetFences(device.getHandle(), 1, &fence);
		}
		vkQueueWaitIdle(queue);
		
		commandBuffer.free(commandPool);
	}
}
