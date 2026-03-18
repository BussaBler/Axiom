#pragma once
#include "axpch.h"
#include "Semaphore.h"
#include "Texture.h"

namespace Axiom {
	class SwapChain {
	public:
		SwapChain() = default;
		virtual ~SwapChain() = default;

		virtual uint32_t acquireNextImage(Semaphore* imageAvailableSemaphore) = 0;
		virtual Texture* getImageTexture(uint32_t index) = 0;
		virtual bool present(uint32_t imageIndex, Semaphore* waitSemaphore) = 0;
		virtual uint32_t getImageCount() const = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
	};
}