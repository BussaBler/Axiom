#include "Renderer.h"

namespace Axiom {
	Renderer* Renderer::instance = nullptr;

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

		renderTargetBarrier = {
			.oldState = TextureState::Undefined,
			.newState = TextureState::RenderTarget,
			.aspect = TextureAspect::Color
		};
		presentBarrier = {
			.oldState = TextureState::RenderTarget,
			.newState = TextureState::Present,
			.aspect = TextureAspect::Color
		};

		createDefaultTexture();
	}

	Renderer::~Renderer() {
		device->waitIdle();
	}

	void Renderer::init(Window* windowPtr) {
		if (!instance) {
			instance = new Renderer(windowPtr);
		}
	}

	void Renderer::shutdown() {
		delete instance;
		instance = nullptr;
	}

	void Renderer::waitIdle() {
		instance->device->waitIdle();
	}

	CommandBuffer* Renderer::beginFrame() {
		instance->inFlightFences[instance->currentFrameIndex]->wait();

		instance->currentImageIndex = instance->swapChain->acquireNextImage(instance->imageAvailableSemaphores[instance->currentFrameIndex].get());
		if (instance->currentImageIndex == std::numeric_limits<uint32_t>::max()) {
			instance->recreateSwapChain();
			return nullptr;
		}

		instance->inFlightFences[instance->currentFrameIndex]->reset();
		CommandBuffer* commandBuffer = instance->commandBuffers[instance->currentFrameIndex].get();
		commandBuffer->begin();
		instance->renderTargetBarrier.texture = instance->swapChain->getImageTexture(instance->currentImageIndex);
		commandBuffer->pipelineBarrier({ instance->renderTargetBarrier });
		return commandBuffer;
	}

	void Renderer::endFrame() {
		CommandBuffer* commandBuffer = instance->commandBuffers[instance->currentFrameIndex].get();
		instance->presentBarrier.texture = instance->renderTargetBarrier.texture;
		commandBuffer->pipelineBarrier({ instance->presentBarrier });
		commandBuffer->end();

		instance->device->submitCommandBuffers({ commandBuffer }, { instance->imageAvailableSemaphores[instance->currentFrameIndex].get() }, { instance->renderFinishedSemaphores[instance->currentImageIndex].get() }, instance->inFlightFences[instance->currentFrameIndex].get());
		bool presentResult = instance->swapChain->present(instance->currentImageIndex, instance->renderFinishedSemaphores[instance->currentImageIndex].get());

		if (!presentResult) {
			instance->recreateSwapChain();
		}

		instance->currentFrameIndex = (instance->currentFrameIndex + 1) % instance->maxFramesInFlight;
	}

	std::unique_ptr<Pipeline> Renderer::createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
		return instance->device->createPipeline(pipelineCreateInfo);
	}

	std::unique_ptr<Buffer> Renderer::createBuffer(const Buffer::CreateInfo& bufferCreateInfo) {
		return instance->device->createBuffer(bufferCreateInfo);
	}

	std::shared_ptr<Texture> Renderer::createTexture(const Texture::CreateInfo& textureCreateInfo) {
		return instance->device->createTexture(textureCreateInfo);
	}

	std::unique_ptr<ResourceLayout> Renderer::createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) {
		return instance->device->createResourceLayout(bindings);
	}

	std::unique_ptr<ResourceSet> Renderer::createResourceSet(ResourceLayout* resourceLayout) {
		return instance->device->createResourceSet(resourceLayout);
	}

	std::unique_ptr<CommandBuffer> Renderer::beginSingleTimeCommands() {
		return instance->device->beginSingleTimeCommands();
	}

	void Renderer::endSingleTimeCommands(CommandBuffer* commandBuffer) {
		instance->device->endSingleTimeCommands(commandBuffer);
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

	void Renderer::createDefaultTexture() {
		const uint32_t defaultTextureSize = 4;

		Texture::CreateInfo textureCreateInfo = {
			.width = defaultTextureSize,
			.height = defaultTextureSize,
			.mipLevels = 1,
			.arrayLayers = 1,
			.format = Format::R8G8B8A8Unorm,
			.usage = TextureUsage::Sampled | TextureUsage::TransferDst,
			.aspect = TextureAspect::Color,
			.initialState = TextureState::Undefined,
			.memoryUsage = MemoryUsage::GPUOnly
		};
		defaultTexture = device->createTexture(textureCreateInfo);


		Buffer::CreateInfo stagingBufferCreateInfo = {
			.size = defaultTextureSize * defaultTextureSize * sizeof(uint32_t),
			.usage = BufferUsage::TransferSrc,
			.memoryUsage = MemoryUsage::GPUandCPU
		};
		std::unique_ptr<Buffer> stagingBuffer = device->createBuffer(stagingBufferCreateInfo);

		// Magenta: R=255, G=0, B=255, A=255
		// Black:   R=0,   G=0, B=0,   A=255
		const uint32_t magenta = 0xFF00FFFF; // AABBGGRR (check your API's expected byte order)
		const uint32_t black = 0xFF000000;

		std::array<uint32_t, defaultTextureSize* defaultTextureSize> defaultTextureData;
		for (uint32_t y = 0; y < defaultTextureSize; y++) {
			for (uint32_t x = 0; x < defaultTextureSize; x++) {
				if ((x + y) % 2 == 0) {
					defaultTextureData[y * defaultTextureSize + x] = magenta;
				}
				else {
					defaultTextureData[y * defaultTextureSize + x] = black;
				}
			}
		}

		std::unique_ptr<CommandBuffer> commandBuffer = device->beginSingleTimeCommands();
		stagingBuffer->setData<uint32_t>(defaultTextureData);
		commandBuffer->copyBufferToTexture(stagingBuffer.get(), defaultTexture.get(), defaultTextureSize, defaultTextureSize);
		device->endSingleTimeCommands(commandBuffer.get());
	}
}
