#include "axpch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"

namespace Axiom {
	VulkanCommandBuffer::~VulkanCommandBuffer() {
		if (state != CommandBufferState::NOT_ALLOCATED) {
			AX_CORE_LOG_ERROR("Destroying a command buffer that is still allocated!");
		}
	}

	void VulkanCommandBuffer::begin(bool isSingleUse, bool isRenderPassCont, bool isSimultaneous) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		if (isSingleUse) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		if (isRenderPassCont) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}
		if (isSimultaneous) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}
		AX_CORE_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS, "Failed to begin recording command buffer!");
		state = CommandBufferState::RECORDING;
	}

	void VulkanCommandBuffer::end() {
		AX_CORE_ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS, "Failed to end recording command buffer!");
		state = CommandBufferState::RECORDING_ENDED;
	}

	void VulkanCommandBuffer::reset() {
		AX_CORE_ASSERT(vkResetCommandBuffer(commandBuffer, 0) == VK_SUCCESS, "Failed to reset command buffer!");
		state = CommandBufferState::READY;
	}

	void VulkanCommandBuffer::allocate(VkCommandPool commandPool, bool isPrimary) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;
		AX_CORE_ASSERT(vkAllocateCommandBuffers(device.getHandle(), &allocInfo, &commandBuffer) == VK_SUCCESS, "Failed to allocate command buffers!");
		state = CommandBufferState::READY;
	}

	void VulkanCommandBuffer::free(VkCommandPool commandPool) {
		vkFreeCommandBuffers(device.getHandle(), commandPool, 1, &commandBuffer);
		commandBuffer = VK_NULL_HANDLE;
		state = CommandBufferState::NOT_ALLOCATED;
	}

	void VulkanCommandBuffer::allocateAndBeginSingleUse(VkCommandPool commandPool, bool isPrimary) {
		allocate(commandPool, isPrimary);
		begin(true, false, false);
	}

	void VulkanCommandBuffer::endSingleUse(VkQueue queue, VkCommandPool commandPool, VkFence fence) {
		end();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		AX_CORE_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence) == VK_SUCCESS, "Failed to submit command buffer!");
		vkQueueWaitIdle(queue);
		free(commandPool);
	}
}
