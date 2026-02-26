#include "axpch.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"

namespace Axiom {
	VulkanContext::VulkanContext(VulkanDevice& vkDevice, VulkanQueue& vkQueue) : device(vkDevice), queue(vkQueue) {

	}

	VulkanContext::~VulkanContext() {
		AX_CORE_LOG_INFO("Destroying Vulkan Context...");
		device.waitIdle();
		for (const auto& frame : frameResources) {
			device.getHandle().destroySemaphore(frame.imageAvailableSemaphore);
			device.getHandle().destroySemaphore(frame.renderFinishedSemaphore);
			device.getHandle().destroyFence(frame.inFlightFence);
		}
		for (const auto& cmdBuffer : mainCommandBuffers) {
			cmdBuffer->free(queue.getCommandPool());
		}
	}

	void VulkanContext::init(uint32_t frameCount) {
		AX_CORE_LOG_INFO("Initializing Vulkan Context with {} frames", frameCount);
		createFrameResources(frameCount);
		createMainCommandBuffer();
	}

	void VulkanContext::submitCommandBuffer(VulkanCommandBuffer& commandBuffer) {
		std::array<Vk::Semaphore, 1> waitSemaphores = { frameResources[currentFrameIndex].imageAvailableSemaphore };
		std::array<Vk::Semaphore, 1> signalSemaphores = { frameResources[currentImageIndex].renderFinishedSemaphore };
		std::array<Vk::PipelineStageFlags, 1> waitStages = { Vk::PipelineStageFlagBits::eAllCommands };
		std::array<Vk::CommandBuffer, 1> commandBuffers = { commandBuffer.getHandle() };

		Vk::SubmitInfo submitInfo(waitSemaphores, waitStages, commandBuffers, signalSemaphores);

		AX_CORE_ASSERT(queue.getHandle().submit(1, &submitInfo, frameResources[currentFrameIndex].inFlightFence) == Vk::Result::eSuccess, "Failed to submit command buffer!");
	}

	bool VulkanContext::begin(Swapchain& swapchain) {
		VulkanSwapchain& vkSwapchain = static_cast<VulkanSwapchain&>(swapchain);
		if (vkSwapchain.getIsRecreating()) {
			device.waitIdle();
			return false;
		}

		VulkanContextFrame& currentFrame = frameResources[currentFrameIndex];
		AX_CORE_ASSERT(device.getHandle().waitForFences(currentFrame.inFlightFence, Vk::True, UINT64_MAX) == Vk::Result::eSuccess, "Failed to wait for fences");

		vkSwapchain.prepare(*this);
		if (vkSwapchain.getIsRecreating()) {
			device.waitIdle();
			return false;
		}
		
		uint32_t imageIndex = vkSwapchain.getCurrentImageIndex();
		if (inFlightFences[imageIndex]) {
			AX_CORE_ASSERT(device.getHandle().waitForFences(currentFrame.inFlightFence, Vk::True, UINT64_MAX) == Vk::Result::eSuccess, "Failed to wait for fences");
		}
		inFlightFences[imageIndex] = frameResources[currentFrameIndex].inFlightFence;

		mainCommandBuffers[imageIndex]->reset();
		mainCommandBuffers[imageIndex]->begin(false, false, false);

		Math::uVec2 extent = vkSwapchain.getExtent();
		Math::Vec2 fExtent = { static_cast<float>(extent.x()), static_cast<float>(extent.y()) };
		Vk::Viewport viewport(0.0f, fExtent.y(), fExtent.x(), -fExtent.y(), 0.0f, 1.0f);
		mainCommandBuffers[imageIndex]->getHandle().setViewport(0, viewport);

		Vk::Rect2D scissor({ 0, 0 }, { extent.x(), extent.y() });
		mainCommandBuffers[imageIndex]->getHandle().setScissor(0, scissor);

		currentImageIndex = imageIndex;
		return true;
	}

	void VulkanContext::end(Swapchain& swapchain) {
		VulkanSwapchain& vkSwapchain = static_cast<VulkanSwapchain&>(swapchain);
		mainCommandBuffers[currentImageIndex]->end();

		device.getHandle().resetFences(frameResources[currentFrameIndex].inFlightFence);

		submitCommandBuffer(*mainCommandBuffers[currentImageIndex]);
	}

	void VulkanContext::bindVertexBuffer(Resource& vertexBuffer, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		VulkanBuffer& vkBuffer = static_cast<VulkanBuffer&>(vertexBuffer);

		std::array<Vk::Buffer, 1> buffers = { vkBuffer.getHandle() };
		std::array<Vk::DeviceSize, 1> offsets = { 0 };

		vkCommandBuffer.getHandle().bindVertexBuffers(0, buffers, offsets);
	}

	void VulkanContext::bindIndexBuffer(Resource& indexBuffer, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		VulkanBuffer& vkBuffer = static_cast<VulkanBuffer&>(indexBuffer);
		
		vkCommandBuffer.getHandle().bindIndexBuffer(vkBuffer.getHandle(), 0, Vk::IndexType::eUint32);
	}

	void VulkanContext::drawIndexed(uint32_t indexCount, uint32_t instanceCount, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);

		vkCommandBuffer.getHandle().drawIndexed(indexCount, instanceCount, 0, 0, 0);
	}

	CommandBuffer& VulkanContext::getMainCommandBuffer() {
		return *mainCommandBuffers[currentImageIndex];
	}

	void VulkanContext::createFrameResources(uint32_t frameCount) {
		frameResources.resize(frameCount);
		inFlightFences.resize(frameCount);
		for (uint32_t i = 0; i < frameCount; i++) {
			VulkanContextFrame frame{};

			Vk::SemaphoreCreateInfo semaphoreInfo{};
			Vk::ResultValue<Vk::Semaphore> imageAvailableSemaphoreResult = device.getHandle().createSemaphore(semaphoreInfo);
			Vk::ResultValue<Vk::Semaphore> renderFinishedSemaphoreResult = device.getHandle().createSemaphore(semaphoreInfo);

			AX_CORE_ASSERT(imageAvailableSemaphoreResult.result == Vk::Result::eSuccess, "Failed to create semaphore!");
			AX_CORE_ASSERT(renderFinishedSemaphoreResult.result == Vk::Result::eSuccess, "Failed to create semaphore!");
			frame.imageAvailableSemaphore = imageAvailableSemaphoreResult.value;
			frame.renderFinishedSemaphore = renderFinishedSemaphoreResult.value;
			
			Vk::FenceCreateInfo fenceInfo(Vk::FenceCreateFlagBits::eSignaled);
			Vk::ResultValue<Vk::Fence> fenceResult = device.getHandle().createFence(fenceInfo);

			AX_CORE_ASSERT(fenceResult.result == Vk::Result::eSuccess, "Failed to create fence!");
			frame.inFlightFence = fenceResult.value;

			frameResources[i] = frame;
		}
	}

	void VulkanContext::createMainCommandBuffer() {
		for (int i = 0; i < frameResources.size(); i++) {
			mainCommandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(device));
			mainCommandBuffers[i]->allocate(device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getCommandPool());
		}
	}
}
