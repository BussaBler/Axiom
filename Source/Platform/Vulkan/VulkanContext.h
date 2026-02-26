#pragma once
#include "Renderer/Context.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;
	class VulkanCommandBuffer;
	class VulkanQueue;
	class VulkanFramebuffer;

	struct VulkanContextFrame {
		Vk::Semaphore imageAvailableSemaphore;
		Vk::Semaphore renderFinishedSemaphore;
		Vk::Fence inFlightFence;
	};

	class VulkanContext : public Context {
	public:
		VulkanContext(VulkanDevice& vkDevice, VulkanQueue& vkQueue);
		~VulkanContext() override;

		void init(uint32_t frameCount);
		void submitCommandBuffer(VulkanCommandBuffer& commandBuffer);
		bool begin(Swapchain& swapchain) override;
		void end(Swapchain& swapchain) override;
		void incrementFrameIndex() override { currentFrameIndex = (currentFrameIndex + 1) % getFrameCount(); }
		void bindVertexBuffer(Resource& vertexBuffer, CommandBuffer& commandBuffer) override;
		void bindIndexBuffer(Resource& indexBuffer, CommandBuffer& commandBuffer) override;
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, CommandBuffer& commandBuffer) override;
		
		uint32_t getCurrentFrameIndex() const override { return currentFrameIndex; }
		VulkanContextFrame getCurrentFrameResource() { return frameResources[currentFrameIndex]; }
		uint32_t getFrameCount() const override { return static_cast<uint32_t>(frameResources.size()); }
		CommandBuffer& getMainCommandBuffer() override;
		Vk::Semaphore getCurrentRenderFinishedSemaphore() const { return frameResources[currentImageIndex].renderFinishedSemaphore; }

	private:
		void createFrameResources(uint32_t frameCount);
		void createMainCommandBuffer();

	private:
		VulkanDevice& device;
		VulkanQueue& queue;
		uint32_t currentFrameIndex = 0;
		uint32_t currentImageIndex = 0;
		std::vector<VulkanContextFrame> frameResources;
		std::vector<Vk::Fence> inFlightFences;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> mainCommandBuffers;
		std::vector<std::unique_ptr<VulkanFramebuffer>> framebuffers;
	};
}

