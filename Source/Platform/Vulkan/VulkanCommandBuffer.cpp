#include "axpch.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& vkDevice) : device(vkDevice), handle(VK_NULL_HANDLE), state(VulkanCommandBufferState::NOT_ALLOCATED) {
		  
	}

	void VulkanCommandBuffer::allocate(VkCommandPool commandPool, VkCommandBufferLevel level) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = level;
		allocInfo.commandBufferCount = 1;
		allocInfo.pNext = nullptr;

		state = VulkanCommandBufferState::READY;
		if (vkAllocateCommandBuffers(device.getHandle(), &allocInfo, &handle) != VK_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to allocate command buffer");
			state = VulkanCommandBufferState::NOT_ALLOCATED;
		}
	}

	void VulkanCommandBuffer::free(VkCommandPool commandPool) {
		if (handle != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(device.getHandle(), commandPool, 1, &handle);
			handle = VK_NULL_HANDLE;
			state = VulkanCommandBufferState::NOT_ALLOCATED;
		}
	}

	void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags usageFlags) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usageFlags;

		vkBeginCommandBuffer(handle, &beginInfo);
		state = VulkanCommandBufferState::RECORDING;
	}

	void VulkanCommandBuffer::end() {
		vkEndCommandBuffer(handle);
		state = VulkanCommandBufferState::RECORDING_ENDED;
	}

	void VulkanCommandBuffer::updateSubmitted() {
		state = VulkanCommandBufferState::SUBMITTED;
	}

	void VulkanCommandBuffer::reset() {
		state = VulkanCommandBufferState::READY;
	}

	void VulkanCommandBuffer::allocateAndBeginSingleUse(VkCommandPool commandPool, VkCommandBufferLevel level) {
		allocate(commandPool, level);
		begin();
	}

	void VulkanCommandBuffer::endAndFreeSingleUse(VkCommandPool commandPool, VkQueue queue) {
		end();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &handle;

		AX_CORE_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, nullptr) == VK_SUCCESS, "Failed to submit Vulkan command buffer");
		vkQueueWaitIdle(queue);

		free(commandPool);
	}
}
