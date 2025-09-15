#pragma once
#include "Renderer/Context.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanCommandBuffer;
	class VulkanQueue;
	class VulkanFramebuffer;

	struct VulkanContextFrame {
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;
	};

	class VulkanContext : public Context {
	public:
		VulkanContext(VulkanDevice& vkDevice, VulkanQueue& vkQueue);
		~VulkanContext() override;

		void init(uint32_t frameCount);
		void submitCommandBuffer(VulkanCommandBuffer& commandBuffer);
		void begin() override;
		void end() override;
		
		uint32_t getCurrentFrameIndex() const override { return currentFrameIndex; }
		VulkanContextFrame getCurrentFrameResource() { return frameResources[currentFrameIndex]; }
		uint32_t getFrameCount() const override { return static_cast<uint32_t>(frameResources.size()); }
		CommandBuffer& getMainCommandBuffer() override;
		VkSemaphore getCurrentRenderFinishedSemaphore() const { return frameResources[currentImageIndex].renderFinishedSemaphore; }
		void setCurrentImageIndex(uint32_t imageIndex) { currentImageIndex = imageIndex; }

	private:
		void createFrameResources(uint32_t frameCount);
		void createMainCommandBuffer();
		void incrementFrameIndex() { currentFrameIndex = (currentFrameIndex + 1) % getFrameCount(); }

	private:
		VulkanDevice& device;
		VulkanQueue& queue;
		uint32_t currentFrameIndex = 0;
		uint32_t currentImageIndex = 0;
		std::vector<VulkanContextFrame> frameResources;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> mainCommandBuffers;
		std::vector<std::unique_ptr<VulkanFramebuffer>> framebuffers;
	};
}

