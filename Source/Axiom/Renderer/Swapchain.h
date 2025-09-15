#pragma once
#include "Context.h"

namespace Axiom {
	struct SwapchainCreateInfo {
		uint32_t desiredFrameCount = 3;
		uint32_t width = 1280;
		uint32_t height = 720;
		void* windowHandle = nullptr;
	};

	class Swapchain {
	public:
		Swapchain(const SwapchainCreateInfo& swapchainCreateInfo) : swapchainCreateInfo(swapchainCreateInfo) {}
		virtual ~Swapchain() = default;

		virtual void rebuild(const SwapchainCreateInfo& swapchainCreateInfo) = 0;
		//virtual void destroy() = 0;
		virtual void present(Context& context) = 0;
		virtual void prepare(Context& context) = 0;

		const SwapchainCreateInfo& getSwapchainCreateInfo() const { return swapchainCreateInfo; }

	protected:
		SwapchainCreateInfo swapchainCreateInfo;
	};
}

