#include "axpch.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanFramebuffer.h"

namespace Axiom {
	VulkanContext::VulkanContext(VulkanDevice& vkDevice, VulkanQueue& vkQueue) : device(vkDevice), queue(vkQueue) {

	}

	VulkanContext::~VulkanContext() {
		AX_CORE_LOG_INFO("Destroying Vulkan Context...");
		vkDeviceWaitIdle(device.getHandle());
		for (const auto& frame : frameResources) {
			vkDestroySemaphore(device.getHandle(), frame.imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(device.getHandle(), frame.renderFinishedSemaphore, nullptr);
			vkDestroyFence(device.getHandle(), frame.inFlightFence, nullptr);
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
		std::array<VkSemaphore, 1> waitSemaphores = { frameResources[currentFrameIndex].imageAvailableSemaphore };
		std::array<VkSemaphore, 1> signalSemaphores = { frameResources[currentImageIndex].renderFinishedSemaphore };
		std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer.getHandlePtr();

		vkQueueSubmit(queue.getHandle(), 1, &submitInfo, frameResources[currentFrameIndex].inFlightFence);
	}

	void VulkanContext::begin() {
		incrementFrameIndex();
		VulkanContextFrame& currentFrame = frameResources[currentFrameIndex];
		vkWaitForFences(device.getHandle(), 1, &currentFrame.inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device.getHandle(), 1, &currentFrame.inFlightFence);

		mainCommandBuffers[currentFrameIndex]->reset();
		mainCommandBuffers[currentFrameIndex]->begin(false, false, false);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 720.0f;
		viewport.width = 1280.0f;
		viewport.height = -720.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mainCommandBuffers[currentFrameIndex]->getHandle(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { 1280, 720 };
		vkCmdSetScissor(mainCommandBuffers[currentFrameIndex]->getHandle(), 0, 1, &scissor);
	}

	void VulkanContext::end() {
		mainCommandBuffers[currentFrameIndex]->end();
		submitCommandBuffer(*mainCommandBuffers[currentFrameIndex]);
	}

	CommandBuffer& VulkanContext::getMainCommandBuffer() {
		return *mainCommandBuffers[currentFrameIndex];
	}

	void VulkanContext::createFrameResources(uint32_t frameCount) {
		frameResources.resize(frameCount);
		for (uint32_t i = 0; i < frameCount; i++) {
			VulkanContextFrame frame{};

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.flags = 0;
			AX_CORE_ASSERT(vkCreateSemaphore(device.getHandle(), &semaphoreInfo, nullptr, &frame.imageAvailableSemaphore) == VK_SUCCESS, "Failed to create semaphore!");
			AX_CORE_ASSERT(vkCreateSemaphore(device.getHandle(), &semaphoreInfo, nullptr, &frame.renderFinishedSemaphore) == VK_SUCCESS, "Failed to create semaphore!");
			
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			AX_CORE_ASSERT(vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &frame.inFlightFence) == VK_SUCCESS, "Failed to create fence!");
			frameResources[i] = frame;
		}
	}

	void VulkanContext::createMainCommandBuffer() {
		for (int i = 0; i < frameResources.size(); i++) {
			mainCommandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(device));
			mainCommandBuffers[i]->allocate(device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getCommandPool());
		}
	}
}
