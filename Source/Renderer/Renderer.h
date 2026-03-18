#pragma once
#include "Device.h"

namespace Axiom {
	class Renderer {
	public:
		Renderer(Window* windowPtr);
		~Renderer();

		static void init(Window* windowPtr);
		static void shutdown();
		static void waitIdle();

		static CommandBuffer* beginFrame();
		static void endFrame();

		static std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo);
		static std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo);
		static std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo);
		static std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings);
		static std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout* resourceLayout);

		static std::unique_ptr<CommandBuffer> beginSingleTimeCommands();
		static void endSingleTimeCommands(CommandBuffer* commandBuffer);

		inline static Texture* getCurrentRenderTarget() {
			return instance->swapChain->getImageTexture(instance->currentImageIndex);
		}
		inline static Math::uVec2 getCurrentRenderTargetSize() {
			return { instance->swapChain->getWidth(), instance->swapChain->getHeight() };
		}
		inline static Texture* getDefaultTexture() {
			return instance->defaultTexture.get();
		}

	private:
		void recreateSwapChain();
		void createDefaultTexture();

	private:
		static Renderer* instance;
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

