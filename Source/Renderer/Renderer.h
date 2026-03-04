#pragma once
#include "Device.h"

namespace Axiom {
	class Renderer {
	public:
		Renderer(Window* windowPtr);
		~Renderer();

		void drawFrame();

	private:
		void recreateSwapChain();

	private:
		static Renderer* instance;
		uint32_t currentFrameIndex = 0;
		uint32_t maxFramesInFlight = 0;
		Window* window = nullptr;
		std::unique_ptr<Device> device = nullptr;
		std::unique_ptr<SwapChain> swapChain = nullptr;
		std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
		std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphores;
		std::vector<std::unique_ptr<Semaphore>> renderFinishedSemaphores;
		std::vector<std::unique_ptr<Fence>> inFlightFences;
		std::unique_ptr<Pipeline> pipeline = nullptr;
		std::unique_ptr<Buffer> vertexBuffer = nullptr;
		std::array<Vertex, 3> vertices{
			Vertex({ 0.0f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }),
			Vertex({ 0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }),
			Vertex({ -0.5f, 0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f })
		};
	};
}

