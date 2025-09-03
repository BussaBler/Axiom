#include "axpch.h"
#include "VulkanBuffer.h"

namespace Axiom {
	VulkanBuffer::VulkanBuffer(VulkanDevice& vkDevice, VkBufferUsageFlags usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind)
		: Buffer(usage, size, memoryPropertyFlags), device(vkDevice) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		handle.emplace<VkBuffer>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateBuffer(device.getHandle<VkDevice>(), &bufferInfo, nullptr, std::any_cast<VkBuffer>(&handle)) == VK_SUCCESS, "Failed to create Vulkan buffer!");
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.getHandle<VkDevice>(), std::any_cast<VkBuffer>(handle), &memRequirements);
		memoryIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryIndex;
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle<VkDevice>(), &allocInfo, nullptr, &memory) == VK_SUCCESS, "Failed to allocate Vulkan buffer memory!");
		
		if (shouldBind) {
			bind(0);
		}
	}

	VulkanBuffer::~VulkanBuffer() {
		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		if (isLocked) {
			unlock();
		}
		vkDestroyBuffer(device.getHandle<VkDevice>(), std::any_cast<VkBuffer>(handle), nullptr);
		vkFreeMemory(device.getHandle<VkDevice>(), memory, nullptr);
	}

	void VulkanBuffer::resize(uint64_t newSize, CommandPool& commandPool, Fence* fence, Queue& queue) {
		if (isLocked) {
			unlock();
		}
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = newSize;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer newHandle;
		AX_CORE_ASSERT(vkCreateBuffer(device.getHandle<VkDevice>(), &bufferInfo, nullptr, &newHandle) == VK_SUCCESS, "Failed to create Vulkan buffer!");
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.getHandle<VkDevice>(), newHandle, &memRequirements);
		memoryIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryIndex;
		VkDeviceMemory newMemory;
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle<VkDevice>(), &allocInfo, nullptr, &newMemory) == VK_SUCCESS, "Failed to allocate Vulkan buffer memory!");
		
		vkBindBufferMemory(device.getHandle<VkDevice>(), newHandle, newMemory, 0);
		// This is the same as the method copyTo but with a VkBuffer instead of a Buffer reference
		vkQueueWaitIdle(queue.getHandle<VkQueue>());
		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocate(commandPool);
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = (std::min)(newSize, totalSize);
		vkCmdCopyBuffer(commandBuffer.getHandle<VkCommandBuffer>(), std::any_cast<VkBuffer>(handle), newHandle, 1, &copyRegion);

		commandBuffer.end();
		VkCommandBuffer commandBufferHandle = commandBuffer.getHandle<VkCommandBuffer>();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBufferHandle;

		if (fence) {
			AX_CORE_ASSERT(vkQueueSubmit(queue.getHandle<VkQueue>(), 1, &submitInfo, fence->getHandle<VkFence>()) == VK_SUCCESS, "Failed to submit Vulkan buffer copy command buffer!");
			vkWaitForFences(device.getHandle<VkDevice>(), 1, fence->getHandlePtr<VkFence>(), VK_TRUE, UINT64_MAX);
			vkResetFences(device.getHandle<VkDevice>(), 1, fence->getHandlePtr<VkFence>());
		}
		else {
			AX_CORE_ASSERT(vkQueueSubmit(queue.getHandle<VkQueue>(), 1, &submitInfo, nullptr) == VK_SUCCESS, "Failed to submit Vulkan buffer copy command buffer!");
		}
		vkQueueWaitIdle(queue.getHandle<VkQueue>());

		commandBuffer.free(commandPool);

		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		vkDestroyBuffer(device.getHandle<VkDevice>(), std::any_cast<VkBuffer>(handle), nullptr);
		vkFreeMemory(device.getHandle<VkDevice>(), memory, nullptr);
		handle = newHandle;
		totalSize = newSize;
		memory = newMemory;
	}

	void VulkanBuffer::bind(uint64_t offset) {
		AX_CORE_ASSERT(vkBindBufferMemory(device.getHandle<VkDevice>(), std::any_cast<VkBuffer>(handle), memory, offset) == VK_SUCCESS, "Failed to bind Vulkan buffer memory!");
	}

	void VulkanBuffer::lock(uint64_t size, uint32_t flags, uint64_t offset) {
		AX_CORE_ASSERT(!isLocked, "Vulkan buffer is already locked!");
		AX_CORE_ASSERT(vkMapMemory(device.getHandle<VkDevice>(), memory, offset, size, flags, &data) == VK_SUCCESS, "Failed to map Vulkan buffer memory!");
		isLocked = true;
	}

	void VulkanBuffer::unlock() {
		AX_CORE_ASSERT(isLocked, "Vulkan buffer is not locked!");
		vkUnmapMemory(device.getHandle<VkDevice>(), memory);
		data = nullptr;
		isLocked = false;
	}

	void VulkanBuffer::copyFrom(const void* srcData, uint64_t size, uint32_t flags, uint64_t offset) {
		void* localData;
		AX_CORE_ASSERT(vkMapMemory(device.getHandle<VkDevice>(), memory, offset, size, flags, &localData) == VK_SUCCESS, "Failed to map Vulkan buffer memory!");
		memcpy(localData, srcData, size);
		vkUnmapMemory(device.getHandle<VkDevice>(), memory);
	}

	void VulkanBuffer::copyTo(Buffer& destination, CommandPool& commandPool, Fence* fence, Queue& queue, uint64_t size, uint64_t srcOffset, uint64_t dstOffset) {
		vkQueueWaitIdle(queue.getHandle<VkQueue>());
		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocate(commandPool);
		commandBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer.getHandle<VkCommandBuffer>(), std::any_cast<VkBuffer>(handle), destination.getHandle<VkBuffer>(), 1, &copyRegion);
		
		commandBuffer.end();
		VkCommandBuffer commandBufferHandle = commandBuffer.getHandle<VkCommandBuffer>();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBufferHandle;
		
		if (fence) {
			AX_CORE_ASSERT(vkQueueSubmit(queue.getHandle<VkQueue>(), 1, &submitInfo, fence->getHandle<VkFence>()) == VK_SUCCESS, "Failed to submit Vulkan buffer copy command buffer!");
			vkWaitForFences(device.getHandle<VkDevice>(), 1, fence->getHandlePtr<VkFence>(), VK_TRUE, UINT64_MAX);
			vkResetFences(device.getHandle<VkDevice>(), 1, fence->getHandlePtr<VkFence>());
		}
		else {
			AX_CORE_ASSERT(vkQueueSubmit(queue.getHandle<VkQueue>(), 1, &submitInfo, nullptr) == VK_SUCCESS, "Failed to submit Vulkan buffer copy command buffer!");
		}
		vkQueueWaitIdle(queue.getHandle<VkQueue>());
		
		commandBuffer.free(commandPool);
	}
}
