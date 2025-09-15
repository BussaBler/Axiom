#pragma once
#include "Context.h"
#include "Swapchain.h"
#include "RenderPassCache.h"

namespace Axiom {
	struct DeviceCreateInfo {

	};

	class Device {
	public:
		virtual ~Device() = default;

		virtual void init(const DeviceCreateInfo& deviceCreateInfo) = 0;
		virtual std::unique_ptr<Context> createContext() = 0;
		virtual std::unique_ptr<Swapchain> createSwapchain(SwapchainCreateInfo& swapchainCreateInfo) = 0;
		virtual std::unique_ptr<RenderPassCache> createRenderPassCache(Swapchain& swapchain) = 0;
	};
}

