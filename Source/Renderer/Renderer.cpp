#include "Renderer.h"

namespace Axiom {
	Renderer::Renderer(Window* windowPtr) : window(windowPtr) {
		Device::CreateInfo deviceCreateInfo{};
		deviceCreateInfo.api = GraphicsApi::Vulkan;
		deviceCreateInfo.windowObjPtr = window;

		device = Device::create(deviceCreateInfo);
		swapChain = device->createSwapchain(window->getWidth(), window->getHeight());

		maxFramesInFlight = 2; // This is a common choice for double buffering

		commandBuffers.resize(maxFramesInFlight);
		imageAvailableSemaphores.resize(maxFramesInFlight);
		inFlightFences.resize(maxFramesInFlight);

		for (size_t i = 0; i < maxFramesInFlight; i++) {
			commandBuffers[i] = device->createCommandBuffer();
			imageAvailableSemaphores[i] = device->createSemaphore();
			inFlightFences[i] = device->createFence(true);
		}

		renderFinishedSemaphores.resize(swapChain->getImageCount());
		for (size_t i = 0; i < swapChain->getImageCount(); i++) {
			renderFinishedSemaphores[i] = device->createSemaphore();
		}
	}

	Renderer::~Renderer() {
		device->waitIdle();
	}

	void Renderer::drawFrame() {
		inFlightFences[currentFrameIndex]->wait();

		uint32_t imageIndex = swapChain->acquireNextImage(imageAvailableSemaphores[currentFrameIndex].get());

		if (imageIndex == std::numeric_limits<uint32_t>::max()) {
			recreateSwapChain();
			return;
		}

		inFlightFences[currentFrameIndex]->reset();

		commandBuffers[currentFrameIndex]->begin();
		Texture::Barrier imageBarrier{
			.texture = swapChain->getImageTexture(imageIndex),
			.oldState = TextureState::Undefined,
			.newState = TextureState::RenderTarget
		};
		commandBuffers[currentFrameIndex]->pipelineBarrier({ imageBarrier });
		RenderAttachment renderAttachment{
			.texture = swapChain->getImageTexture(imageIndex),
			.loadOp = LoadOp::Clear,
			.storeOp = StoreOp::Store,
			.clearColor = Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f)
		};
		RenderPass renderPass{
			.colorAttachments = { renderAttachment },
			.colorAttachmentCount = 1,
			.width = swapChain->getWidth(),
			.height = swapChain->getHeight()
		};
		commandBuffers[currentFrameIndex]->beginRendering(renderPass);
		commandBuffers[currentFrameIndex]->endRendering();
		Texture::Barrier presentBarrier{
			.texture = swapChain->getImageTexture(imageIndex),
			.oldState = TextureState::RenderTarget,
			.newState = TextureState::Present
		};
		commandBuffers[currentFrameIndex]->pipelineBarrier({ presentBarrier });
		commandBuffers[currentFrameIndex]->end();

		device->submitCommandBuffers({ commandBuffers[currentFrameIndex].get() }, { imageAvailableSemaphores[currentFrameIndex].get() }, { renderFinishedSemaphores[imageIndex].get() }, inFlightFences[currentFrameIndex].get());
		bool presentResult = swapChain->present(imageIndex, renderFinishedSemaphores[imageIndex].get());

		if (!presentResult) {
			recreateSwapChain();
		}

		currentFrameIndex = (currentFrameIndex + 1) % maxFramesInFlight;
	}

	void Renderer::recreateSwapChain() {
		uint32_t width = window->getWidth();
		uint32_t height = window->getHeight();

		while (width == 0 || height == 0) {
			window->onUpdate();
			width = window->getWidth();
			height = window->getHeight();
		}

		device->waitIdle();
		swapChain.reset();
		swapChain = device->createSwapchain(width, height);

		renderFinishedSemaphores.clear();
		renderFinishedSemaphores.resize(swapChain->getImageCount());
		for (size_t i = 0; i < swapChain->getImageCount(); i++) {
			renderFinishedSemaphores[i] = device->createSemaphore();
		}
	}
}
