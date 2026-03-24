#pragma once
#include "Device.h"

namespace Axiom {
	class Renderer {
	public:
		Renderer(Window* windowPtr);
		~Renderer();

		void waitIdle();

		CommandBuffer* beginFrame();
		void endFrame();

		std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo);
		std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo);
		std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo);
		std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings);
		std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout* resourceLayout);

		std::unique_ptr<CommandBuffer> beginSingleTimeCommands();
		void endSingleTimeCommands(CommandBuffer* commandBuffer);

		inline Texture* getCurrentRenderTarget() {
			return swapChain->getImageTexture(currentImageIndex);
		}
		inline Math::uVec2 getCurrentRenderTargetSize() {
			return { swapChain->getWidth(), swapChain->getHeight() };
		}
		inline Texture* getDefaultTexture() {
			return defaultTexture.get();
		}

	private:
		void recreateSwapChain();
		void createDefaultTexture();

	private:
		uint32_t currentFrameIndex = 0;
		uint32_t maxFramesInFlight = 0;
		uint32_t currentImageIndex = 0;
		Window* window = nullptr;
		std::unique_ptr<Device> device = nullptr;
		std::unique_ptr<SwapChain> swapChain = nullptr;
		std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
		std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphores;
		std::vector<std::unique_ptr<Semaphore>> renderFinishedSemaphores;
		std::vector<std::unique_ptr<Fence>> inFlightFences;
		Texture::Barrier renderTargetBarrier;
		Texture::Barrier presentBarrier;
		std::shared_ptr<Texture> defaultTexture = nullptr;
	};
}

