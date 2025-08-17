#pragma once
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

	class VulkanCommandBuffer {
	public:
		VulkanCommandBuffer(VulkanDevice& vkDevice);
		~VulkanCommandBuffer() = default;

		void allocate(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		void free(VkCommandPool commandPool);
		void begin(VkCommandBufferUsageFlags usageFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		void updateSubmitted();
		void reset();
		// Convenience method to allocate and begin recording in one step
		void allocateAndBeginSingleUse(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		// Ends recording, submits and waits for the queue to become idle, and frees the command buffer
		void endAndFreeSingleUse(VkCommandPool commandPool, VkQueue queue);

		VkCommandBuffer getHandle() const { return handle; }
		void setState(VulkanCommandBufferState inState) { state = inState; }
		VulkanCommandBufferState getState() const { return state; }

	private:
		VulkanDevice& device;
		VkCommandBuffer handle;
		VulkanCommandBufferState state;
	};
}

