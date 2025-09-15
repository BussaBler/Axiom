#pragma once
#include "Core/Log.h"
#include "Renderer/CommandBuffer.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanQueue;

	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(VulkanDevice& vkDevice) : device(vkDevice), commandBuffer(VK_NULL_HANDLE) {}
		~VulkanCommandBuffer() override;

		void begin(bool isSingleUse, bool isRenderPassCont, bool isSimultaneous) override;
		void end() override;
		void reset() override;

		void allocate(VkCommandPool commandPool, bool isPrimary = true);
		void free(VkCommandPool commandPool);

		void allocateAndBeginSingleUse(VkCommandPool commandPool, bool isPrimary = true);

		VkCommandBuffer getHandle() const { return commandBuffer; }
		VkCommandBuffer* getHandlePtr() { return &commandBuffer; }

	private:
		VulkanDevice& device;
		VkCommandBuffer commandBuffer;
	};
}

