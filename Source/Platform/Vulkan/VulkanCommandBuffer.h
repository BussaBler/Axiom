#pragma once
#include "Renderer/Core/CommandBuffer.h"
#include "VulkanDevice.h"

namespace Axiom {
	enum class VulkanCommandBufferState {
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMITTED,
		NOT_ALLOCATED
	};

	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(VulkanDevice& vkDevice);
		~VulkanCommandBuffer() = default;

		void allocate(CommandPool& commandPool, bool primary = true) override;
		void free(CommandPool& commandPool) override;
		void begin(uint32_t usageFlags = 0) override;
		void end() override;
		void updateSubmitted() override;
		void reset() override;
		void allocateAndBeginSingleUse(CommandPool& commandPool, bool primary = true) override;
		void endAndFreeSingleUse(CommandPool& commandPool, Queue& queue) override;

		void setState(VulkanCommandBufferState inState) { state = inState; }
		VulkanCommandBufferState getState() const { return state; }

	private:
		VulkanDevice& device;
		VulkanCommandBufferState state;
	};
}

