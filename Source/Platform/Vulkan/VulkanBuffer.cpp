#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanBuffer::~VulkanBuffer() {
		if (buffer != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(device.getHandle());
			vkDestroyBuffer(device.getHandle(), buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
	}

	void VulkanBuffer::init(ResourceCreateInfo& createInfo) {
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = createInfo.size;
		bufferCreateInfo.usage = getBufferUsage(createInfo.usage);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		AX_CORE_ASSERT(vkCreateBuffer(device.getHandle(), &bufferCreateInfo, nullptr, &buffer) == VK_SUCCESS, "Failed to create Vulkan Buffer!");
	
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device.getHandle(), buffer, &memRequirements);

		memorySize = memRequirements.size;
		uint32_t memoryTypeIndex = device.getAdapter().findMemoryType(memRequirements.memoryTypeBits, getMemoryPropertyFlags(createInfo.memoryUsage));
		
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle(), &allocInfo, nullptr, &memory) == VK_SUCCESS, "Failed to allocate Vulkan Buffer memory!");
		bind(0);
	}

	void VulkanBuffer::lockMemory(void** mappedMemory, uint64_t offset, uint64_t size, uint32_t flags) {
		if (locked) {
			AX_CORE_LOG_WARN("Vulkan Buffer memory is already locked!");
			return;
		}
		if (size == VK_WHOLE_SIZE) {
			size = memorySize;
		}
		AX_CORE_ASSERT(vkMapMemory(device.getHandle(), memory, offset, size, 0, mappedMemory) == VK_SUCCESS, "Failed to map Vulkan Buffer memory!");
		locked = true;
	}

	void VulkanBuffer::unlockMemory() {
		if (!locked) {
			AX_CORE_LOG_WARN("Vulkan Buffer memory is not locked!");
			return;
		}
		vkUnmapMemory(device.getHandle(), memory);
		locked = false;
	}

	void VulkanBuffer::loadData(void* data, uint64_t size, uint64_t offset, uint32_t flags) {
		void* mappedMemory = nullptr;
		lockMemory(&mappedMemory, offset, size, flags);
		memcpy(mappedMemory, data, size);
		unlockMemory();
	}

	void VulkanBuffer::copyToBuffer(VulkanBuffer& destination, VkCommandPool commandPool, VkFence fence, VkQueue queue, uint64_t srcOffset, uint64_t dstOffset, uint64_t size) {
		vkQueueWaitIdle(queue);

		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocateAndBeginSingleUse(commandPool, true);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = (size == VK_WHOLE_SIZE) ? memorySize : size;
		vkCmdCopyBuffer(commandBuffer.getHandle(), buffer, destination.getHandle(), 1, &copyRegion);
		commandBuffer.endSingleUse(queue, commandPool, fence);
	}

	void VulkanBuffer::bind(uint64_t offset) {
		AX_CORE_ASSERT(vkBindBufferMemory(device.getHandle(), buffer, memory, offset) == VK_SUCCESS, "Failed to bind Vulkan Buffer memory!");
	}

	VkBufferUsageFlags VulkanBuffer::getBufferUsage(uint32_t usage) {
		VkBufferUsageFlags bufferUsage = 0;
		if (usage & ResourceUsage::VertexBuffer) {
			bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (usage & ResourceUsage::IndexBuffer) {
			bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (usage & ResourceUsage::UniformBuffer) {
			bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (usage & ResourceUsage::ShaderResource) {
			bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (usage & ResourceUsage::TransferDst) {
			bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		if (usage & ResourceUsage::TransferSrc) {
			bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (usage & ResourceUsage::CopyDst) {
			bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		if (usage & ResourceUsage::CopySrc) {
			bufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		return bufferUsage;
	}
}