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
		Vk::CommandBufferUsageFlags usageFlags = Vk::CommandBufferUsageFlags();
		if (isSingleUse) {
			usageFlags |= Vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		}
		if (isRenderPassCont) {
			usageFlags |= Vk::CommandBufferUsageFlagBits::eRenderPassContinue;
		}
		if (isSimultaneous) {
			usageFlags |= Vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		}

		Vk::CommandBufferBeginInfo beginInfo(usageFlags);
		AX_CORE_ASSERT(commandBuffer.begin(beginInfo) == Vk::Result::eSuccess, "Failed to begin recording command buffer!");
		state = CommandBufferState::RECORDING;
	}

	void VulkanCommandBuffer::end() {
		AX_CORE_ASSERT(commandBuffer.end() == Vk::Result::eSuccess, "Failed to end recording command buffer!");
		state = CommandBufferState::RECORDING_ENDED;
	}

	void VulkanCommandBuffer::reset() {
		AX_CORE_ASSERT(commandBuffer.reset() == Vk::Result::eSuccess, "Failed to reset command buffer!");
		state = CommandBufferState::READY;
	}

	void VulkanCommandBuffer::allocate(Vk::CommandPool commandPool, bool isPrimary) {
		Vk::CommandBufferLevel level = isPrimary ? Vk::CommandBufferLevel::ePrimary : Vk::CommandBufferLevel::eSecondary;
		Vk::CommandBufferAllocateInfo allocInfo(commandPool, level, 1);

		Vk::ResultValue<std::vector<Vk::CommandBuffer>> allocResult = device.getHandle().allocateCommandBuffers(allocInfo);

		AX_CORE_ASSERT(allocResult.result == Vk::Result::eSuccess, "Failed to allocate command buffers!");
		commandBuffer = allocResult.value[0];
		state = CommandBufferState::READY;
	}

	void VulkanCommandBuffer::free(Vk::CommandPool commandPool) {
		device.getHandle().freeCommandBuffers(commandPool, commandBuffer);
		commandBuffer = VK_NULL_HANDLE;
		state = CommandBufferState::NOT_ALLOCATED;
	}

	void VulkanCommandBuffer::allocateAndBeginSingleUse(Vk::CommandPool commandPool, bool isPrimary) {
		allocate(commandPool, isPrimary);
		begin(true, false, false);
	}

	void VulkanCommandBuffer::endSingleUse(Vk::Queue queue, Vk::CommandPool commandPool, Vk::Fence fence) {
		end();
		Vk::SubmitInfo submitInfo({}, {}, commandBuffer);
		AX_CORE_ASSERT(queue.submit(1, &submitInfo, fence) == Vk::Result::eSuccess, "Failed to submit command buffer!");
		
		AX_CORE_ASSERT(queue.waitIdle() == Vk::Result::eSuccess, "Failed to wait for queue idle after submitting command buffer!");
		free(commandPool);
	}
}
