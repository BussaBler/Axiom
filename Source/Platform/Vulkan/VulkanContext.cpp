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

	bool VulkanContext::begin(Swapchain& swapchain) {
		VulkanSwapchain& vkSwapchain = static_cast<VulkanSwapchain&>(swapchain);
		if (vkSwapchain.getIsRecreating()) {
			vkDeviceWaitIdle(device.getHandle());
			return false;
		}

		VulkanContextFrame& currentFrame = frameResources[currentFrameIndex];
		vkWaitForFences(device.getHandle(), 1, &currentFrame.inFlightFence, VK_TRUE, UINT64_MAX);

		vkSwapchain.prepare(*this);
		if (vkSwapchain.getIsRecreating()) {
			vkDeviceWaitIdle(device.getHandle());
			return false;
		}
		
		uint32_t imageIndex = vkSwapchain.getCurrentImageIndex();
		if (inFlightFences[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device.getHandle(), 1, &inFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
		}
		inFlightFences[imageIndex] = frameResources[currentFrameIndex].inFlightFence;

		mainCommandBuffers[imageIndex]->reset();
		mainCommandBuffers[imageIndex]->begin(false, false, false);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 720.0f;
		viewport.width = 1280.0f;
		viewport.height = -720.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mainCommandBuffers[imageIndex]->getHandle(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { 1280, 720 };
		vkCmdSetScissor(mainCommandBuffers[imageIndex]->getHandle(), 0, 1, &scissor);

		currentImageIndex = imageIndex;
		return true;
	}

	void VulkanContext::end(Swapchain& swapchain) {
		VulkanSwapchain& vkSwapchain = static_cast<VulkanSwapchain&>(swapchain);
		mainCommandBuffers[currentImageIndex]->end();

		vkResetFences(device.getHandle(), 1, &frameResources[currentFrameIndex].inFlightFence);

		submitCommandBuffer(*mainCommandBuffers[currentImageIndex]);
	}

	void VulkanContext::bindVertexBuffer(Resource& vertexBuffer, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		VulkanBuffer& vkBuffer = static_cast<VulkanBuffer&>(vertexBuffer);

		std::array<VkBuffer, 1> buffers = { vkBuffer.getHandle() };
		std::array<VkDeviceSize, 1> offsets = { 0 };

		vkCmdBindVertexBuffers(vkCommandBuffer.getHandle(), 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}

	void VulkanContext::bindIndexBuffer(Resource& indexBuffer, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		VulkanBuffer& vkBuffer = static_cast<VulkanBuffer&>(indexBuffer);
		vkCmdBindIndexBuffer(vkCommandBuffer.getHandle(), vkBuffer.getHandle(), 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanContext::drawIndexed(uint32_t indexCount, uint32_t instanceCount, CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCmdDrawIndexed(vkCommandBuffer.getHandle(), indexCount, instanceCount, 0, 0, 0);
	}

	CommandBuffer& VulkanContext::getMainCommandBuffer() {
		return *mainCommandBuffers[currentImageIndex];
	}

	void VulkanContext::createFrameResources(uint32_t frameCount) {
		frameResources.resize(frameCount);
		inFlightFences.resize(frameCount);
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
