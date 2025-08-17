#include "axpch.h"
#include "VulkanContext.h"

namespace Axiom {
	void VulkanContext::init(Window* window) {
		this->window = window;
		framebufferWidth = window->getWidth();
		framebufferHeight = window->getHeight();
		createDevice();
		createSwapChain();
		createRenderPass();
		createFramebuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	void VulkanContext::shutdown() {
		vkDeviceWaitIdle(device->getHandle());

		for (size_t i = 0; i < swapchain->getImageCount(); i++) {
			vkDestroySemaphore(device->getHandle(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device->getHandle(), queueCompleteSemaphores[i], nullptr);
		}
	}

	bool VulkanContext::beginFrame() {
		if (recreatingSwapChain) {
			VkResult result = vkDeviceWaitIdle(device->getHandle());
			if (result != VK_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to wait for device idle (1): {}", static_cast<int>(result));
			}
			return false;
		}
		if (framebufferGen != lastFramebufferGen) {
			AX_CORE_LOG_INFO("Recreating Vulkan swap chain due to framebuffer generation change");
			VkResult result = vkDeviceWaitIdle(device->getHandle());
			if (result != VK_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to wait for device idle (2): {}", static_cast<int>(result));
			}
			recreateSwapChain();
			return false;
		}

		inFlightFences[currentFrame]->wait(UINT64_MAX);
		swapchain->acquireNextImageIndex(imageIndex, imageAvailableSemaphores[currentFrame], nullptr, { framebufferWidth, framebufferHeight });
		graphicsCommandBuffers[imageIndex]->reset();
		graphicsCommandBuffers[imageIndex]->begin(0);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(framebufferHeight);
		viewport.width = static_cast<float>(framebufferWidth);
		viewport.height = -static_cast<float>(framebufferHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { framebufferWidth, framebufferHeight };

		vkCmdSetViewport(graphicsCommandBuffers[imageIndex]->getHandle(), 0, 1, &viewport);
		vkCmdSetScissor(graphicsCommandBuffers[imageIndex]->getHandle(), 0, 1, &scissor);

		mainRenderPass->setWidth(static_cast<float>(framebufferWidth));
		mainRenderPass->setHeight(static_cast<float>(framebufferHeight));
		mainRenderPass->begin(*graphicsCommandBuffers[imageIndex], framebuffers[imageIndex]->getHandle());

		return true;
	}

	bool VulkanContext::endFrame() {
		mainRenderPass->end(*graphicsCommandBuffers[imageIndex]);
		graphicsCommandBuffers[imageIndex]->end();

		if (imagesInFlightFences[imageIndex]) {
			imagesInFlightFences[imageIndex]->wait(UINT64_MAX);
		}

		imagesInFlightFences[imageIndex] = inFlightFences[currentFrame].get();
		inFlightFences[currentFrame]->reset();

		VkCommandBuffer commandBuffer = graphicsCommandBuffers[imageIndex]->getHandle();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &queueCompleteSemaphores[imageIndex];
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
		
		std::array<VkPipelineStageFlags, 1> pipelineStages{};
		pipelineStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submitInfo.pWaitDstStageMask = pipelineStages.data();
		VkResult result = vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]->getHandle());
		if (result != VK_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to submit draw command buffer: {}", static_cast<int>(result));
		}

		graphicsCommandBuffers[imageIndex]->updateSubmitted();

		swapchain->presentImage(device->getGraphicsQueue(), device->getPresentQueue(), queueCompleteSemaphores[imageIndex], imageIndex, currentFrame, { framebufferWidth, framebufferHeight });
		
		return true;
	}

	void VulkanContext::onResize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) return;
		AX_CORE_LOG_INFO("Resizing Vulkan swap chain to {}x{}", width, height);
		framebufferWidth = width;
		framebufferHeight = height;
		framebufferGen++;
	}

	void VulkanContext::createDevice() {
		AX_CORE_LOG_INFO("Creating Vulkan device");
		device = std::make_unique<VulkanDevice>(window);
	}

	void VulkanContext::createSwapChain() {
		VkExtent2D extent = { framebufferWidth, framebufferHeight };
		AX_CORE_LOG_INFO("Creating Vulkan swap chain with extent: {}x{}", extent.width, extent.height);
		swapchain = std::make_unique<VulkanSwapChain>(*device, extent);
	}

	void VulkanContext::recreateSwapChain() {
		if (recreatingSwapChain) return;
		recreatingSwapChain = true;
		AX_CORE_LOG_INFO("Recreating Vulkan swap chain");
		swapchain->recreate({ framebufferWidth, framebufferHeight });
		createFramebuffer();
		createCommandBuffers();
		recreatingSwapChain = false;
		lastFramebufferGen = framebufferGen;
	}

	void VulkanContext::createRenderPass() {
		AX_CORE_LOG_INFO("Creating Vulkan main render pass");
		mainRenderPass = std::make_unique<VulkanRenderPass>(*device, swapchain.get(),
			0.0f, 0.0f, framebufferWidth, framebufferHeight, 0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0);
	}

	void VulkanContext::createCommandBuffers() {
		AX_CORE_LOG_INFO("Creating Vulkan command buffers");
		graphicsCommandBuffers.resize(swapchain->getImageCount());
		for (size_t i = 0; i < graphicsCommandBuffers.size(); i++) {
			graphicsCommandBuffers[i] = std::make_unique<VulkanCommandBuffer>(*device);
			graphicsCommandBuffers[i]->allocate(device->getGraphicsCommandPool());
		}
	}

	void VulkanContext::createFramebuffer() {
		AX_CORE_LOG_INFO("Creating Vulkan framebuffers");
		auto& views = swapchain->getImageViews();
		framebuffers.resize(swapchain->getImageCount());
		for (uint32_t i = 0; i < swapchain->getImageCount(); i++) {
			std::vector<VkImageView> attachments = {
				views[i],
				swapchain->getDepthImage().getImageView()
			};
			framebuffers[i] = std::make_unique<VulkanFramebuffer>(*device, *mainRenderPass, attachments, framebufferWidth, framebufferHeight);
		}
	}

	void VulkanContext::createSyncObjects() {
		AX_CORE_LOG_INFO("Creating Vulkan synchronization objects");
		imageAvailableSemaphores.resize(swapchain->getImageCount());
		queueCompleteSemaphores.resize(swapchain->getImageCount());
		inFlightFences.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
		imagesInFlightFences.resize(swapchain->getImageCount());

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < swapchain->getImageCount(); i++) {
			if (vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device->getHandle(), &semaphoreInfo, nullptr, &queueCompleteSemaphores[i]) != VK_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to create semaphore(s) for a frame!");
			}
			imagesInFlightFences[i] = nullptr;
		}
		for (size_t i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			inFlightFences[i] = std::make_unique<VulkanFence>(*device, true);
		}
	}
}
