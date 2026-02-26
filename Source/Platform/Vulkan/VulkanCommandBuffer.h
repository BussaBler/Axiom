#pragma once
#include "Core/Log.h"
#include "Renderer/CommandBuffer.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;

	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(VulkanDevice& vkDevice) : device(vkDevice), commandBuffer(VK_NULL_HANDLE) {}
		~VulkanCommandBuffer() override;

		void begin(bool isSingleUse, bool isRenderPassCont, bool isSimultaneous) override;
		void end() override;
		void reset() override;

		void allocate(Vk::CommandPool commandPool, bool isPrimary = true);
		void free(Vk::CommandPool commandPool);

		void allocateAndBeginSingleUse(Vk::CommandPool commandPool, bool isPrimary = true);
		void endSingleUse(Vk::Queue queue, Vk::CommandPool commandPool, Vk::Fence fence = VK_NULL_HANDLE);

		Vk::CommandBuffer getHandle() const { return commandBuffer; }
		Vk::CommandBuffer* getHandlePtr() { return &commandBuffer; }

	private:
		VulkanDevice& device;
		Vk::CommandBuffer commandBuffer;
	};
}

