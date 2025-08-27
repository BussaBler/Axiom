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
		createShaders();
		createBuffers();
		// Temp
		float f = 10.0f;
		std::vector<Vertex> vertices(4, {});
		vertices[0].position = { -0.5f * f, -0.5f * f, 0.0f };
		vertices[0].texCoord = { 0.0f, 0.0f };
		vertices[1].position = { 0.5f * f, 0.5f * f, 0.0f };
		vertices[1].texCoord = { 1.0f, 1.0f };
		vertices[2].position = { -0.5f * f, 0.5f * f, 0.0f };
		vertices[2].texCoord = { 0.0f, 1.0f };
		vertices[3].position = { 0.5f * f, -0.5f * f, 0.0f };
		vertices[3].texCoord = { 1.0f, 0.0f };
		std::vector<uint32_t> indices = { 0, 1, 2, 0, 3, 1 };

		uploadData(device->getGraphicsCommandPool(), nullptr, device->getGraphicsQueue(), *objectVertexBuffer, vertices.data(), sizeof(Vertex) * vertices.size());
		uploadData(device->getGraphicsCommandPool(), nullptr, device->getGraphicsQueue(), *objectIndexBuffer, indices.data(), sizeof(uint32_t) * indices.size());
	
		uint32_t objectId = objectShader->acquireResources();
	}

	void VulkanContext::shutdown() {
		vkDeviceWaitIdle(device->getHandle());

		for (size_t i = 0; i < swapchain->getImageCount(); i++) {
			vkDestroySemaphore(device->getHandle(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device->getHandle(), queueCompleteSemaphores[i], nullptr);
		}
	}

	bool VulkanContext::beginFrame(float deltaTime) {
		frameDeltaTime = deltaTime;
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

	void VulkanContext::updateGlobalState(Math::Mat4 projection, Math::Mat4 view, Math::Vec3 viewPos, Math::Vec4 ambientColor, int mode) {
		objectShader->use(*graphicsCommandBuffers[imageIndex]);

		GlobalUniformObject ubo(projection, view, Math::Mat4::identity(), Math::Mat4::identity());
		objectShader->updateGlobalUniformBuffer(ubo);

		objectShader->updateGlobalUniformBufferState(*graphicsCommandBuffers[imageIndex], imageIndex, frameDeltaTime);
	}

	void VulkanContext::updateObjectState(const GeometryRenderData& data) {
		objectShader->updateObjectUniformBufferState(*graphicsCommandBuffers[imageIndex], imageIndex, data);

		// Temp
		objectShader->use(*graphicsCommandBuffers[imageIndex]);
		std::array<VkDeviceSize, 1> offsets = { 0 };
		std::array<VkBuffer, 1> buffers = { objectVertexBuffer->getHandle() };
		vkCmdBindVertexBuffers(graphicsCommandBuffers[imageIndex]->getHandle(), 0, 1, buffers.data(), offsets.data());
		vkCmdBindIndexBuffer(graphicsCommandBuffers[imageIndex]->getHandle(), objectIndexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(graphicsCommandBuffers[imageIndex]->getHandle(), 6, 1, 0, 0, 0);
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

	std::shared_ptr<Texture> VulkanContext::createTexture(uint32_t width, uint32_t height, uint8_t channels, std::vector<uint8_t> data) {
		return std::make_shared<VulkanTexture>(*device, width, height, channels, data);
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
				swapchain->getDepthImage()->getImageView()
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

	void VulkanContext::createShaders() {
		AX_CORE_LOG_INFO("Creating Vulkan builtin shaders");
		objectShader = std::make_unique<VulkanObjectShader>(*device);
		objectShader->createPipeline(*mainRenderPass, framebufferWidth, framebufferHeight);
	}

	void VulkanContext::createBuffers() {
		AX_CORE_LOG_INFO("Creating Vulkan buffers");
		VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		uint64_t vertexBufferSize = sizeof(Vertex) * 1024 * 1024;
		objectVertexBuffer = std::make_unique<VulkanBuffer>(*device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertexBufferSize, memoryFlags, true);
		geometryVertexOffset = 0;

		uint64_t indexBufferSize = sizeof(uint32_t) * 1024 * 1024;
		objectIndexBuffer = std::make_unique<VulkanBuffer>(*device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indexBufferSize, memoryFlags, true);
		geometryIndexOffset = 0;
	}

	void VulkanContext::uploadData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer& buffer, void* data, uint64_t size, uint64_t offset) {
		VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VulkanBuffer stagingBuffer(*device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, flags, true);
		stagingBuffer.copyFrom(data, size, 0, offset);
		stagingBuffer.copyTo(buffer.getHandle(), pool, fence, queue, size, 0, offset);
	}
}
