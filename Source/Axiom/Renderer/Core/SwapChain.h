#pragma once
#include "Fence.h"
#include "Semaphore.h"
#include "Queue.h"
#include "Math/AxMath.h"

namespace Axiom {
	class SwapChain {
	public:
		SwapChain() = default;
		virtual ~SwapChain() = default;

		virtual void recreate(Math::uVec2 extent) = 0;
		virtual void acquireNextImageIndex(uint32_t& imageIndex, const Semaphore& semaphore, Fence* fence, Math::uVec2 extent) = 0;
		virtual void presentImage(const Queue& graphicsQueue, const Queue& presentQueue, const Semaphore& renderSemaphore, uint32_t imageIndex, uint32_t& currentFrame, Math::uVec2 extent) = 0;
	
		template<typename T>	
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<SwapChain> create(Device& deviceRef, Math::iVec2 extent);

	protected:
		std::any handle;
	};
}

