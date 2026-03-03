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
	};
}

