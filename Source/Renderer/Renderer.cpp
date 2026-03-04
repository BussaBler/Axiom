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

		Pipeline::CreateInfo pipelineCreateInfo = {
			.vertexShaderPath = "Assets/Shaders/shader.vert",
			.fragmentShaderPath = "Assets/Shaders/shader.frag",
			.topology = PrimitiveTopology::TriangleList,
			.polygonMode = PolygonMode::Fill,
			.cullMode = CullMode::Disabled,
			.frontFaceClockwise = true,
			.enableBlending = false,
			.enableDepthTest = false,
			.enableDepthWrite = false,
			.colorAttachmentFormats = { swapChain->getImageTexture(0)->getFormat() },
			.depthAttachmentFormat = Format::Undefined
		};

		pipeline = device->createPipeline(pipelineCreateInfo);

		Buffer::CreateInfo vertexBufferCreateInfo = {
			.size = sizeof(Vertex) * vertices.size(),
			.usage = BufferUsage::Vertex,
			.memoryUsage = MemoryUsage::GPUandCPU
		};
		vertexBuffer = device->createBuffer(vertexBufferCreateInfo);
		void* vertexData = vertexBuffer->map();
		memcpy(vertexData, vertices.data(), sizeof(Vertex) * vertices.size());
		vertexBuffer->unmap();
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
		commandBuffers[currentFrameIndex]->bindPipeline(pipeline.get());
		commandBuffers[currentFrameIndex]->setViewport(0.0f, 0.0f, static_cast<float>(swapChain->getWidth()), static_cast<float>(swapChain->getHeight()));
		commandBuffers[currentFrameIndex]->setScissor(0, 0, swapChain->getWidth(), swapChain->getHeight());
		commandBuffers[currentFrameIndex]->bindVertexBuffers({ vertexBuffer.get() });
		commandBuffers[currentFrameIndex]->draw(static_cast<uint32_t>(vertices.size()), 1);
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
