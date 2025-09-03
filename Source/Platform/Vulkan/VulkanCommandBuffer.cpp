#include "axpch.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& vkDevice) : device(vkDevice), state(VulkanCommandBufferState::NOT_ALLOCATED) {
		  
	}

	void VulkanCommandBuffer::allocate(CommandPool& commandPool, bool primary) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool.getHandle<VkCommandPool>();
		allocInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.pNext = nullptr;

		state = VulkanCommandBufferState::READY;
		handle.emplace<VkCommandBuffer>(VK_NULL_HANDLE);
		if (vkAllocateCommandBuffers(device.getHandle<VkDevice>(), &allocInfo, std::any_cast<VkCommandBuffer>(&handle)) != VK_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to allocate command buffer");
			state = VulkanCommandBufferState::NOT_ALLOCATED;
		}
	}

	void VulkanCommandBuffer::free(CommandPool& commandPool) {
		if (getHandle<VkCommandBuffer>() != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(device.getHandle<VkDevice>(), commandPool.getHandle<VkCommandPool>(), 1, std::any_cast<VkCommandBuffer>(&handle));
			handle = VK_NULL_HANDLE;
			state = VulkanCommandBufferState::NOT_ALLOCATED;
		}
	}

	void VulkanCommandBuffer::begin(uint32_t usageFlags) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usageFlags;

		vkBeginCommandBuffer(getHandle<VkCommandBuffer>(), &beginInfo);
		state = VulkanCommandBufferState::RECORDING;
	}

	void VulkanCommandBuffer::end() {
		vkEndCommandBuffer(getHandle<VkCommandBuffer>());
		state = VulkanCommandBufferState::RECORDING_ENDED;
	}

	void VulkanCommandBuffer::updateSubmitted() {
		state = VulkanCommandBufferState::SUBMITTED;
	}

	void VulkanCommandBuffer::reset() {
		state = VulkanCommandBufferState::READY;
	}

	void VulkanCommandBuffer::allocateAndBeginSingleUse(CommandPool& commandPool, bool primary) {
		allocate(commandPool, primary);
		begin();
	}

	void VulkanCommandBuffer::endAndFreeSingleUse(CommandPool& commandPool, Queue& queue) {
		end();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = std::any_cast<VkCommandBuffer>(&handle);

		AX_CORE_ASSERT(vkQueueSubmit(queue.getHandle<VkQueue>(), 1, &submitInfo, nullptr) == VK_SUCCESS, "Failed to submit Vulkan command buffer");
		vkQueueWaitIdle(queue.getHandle<VkQueue>());

		free(commandPool);
	}
}
