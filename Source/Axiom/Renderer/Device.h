#pragma once
#include "Context.h"
#include "Swapchain.h"
#include "RenderPassCache.h"
#include "Shader.h"
#include "Resource.h"

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
		virtual std::unique_ptr<Shader> createShader(RenderPassToken& token) = 0;
		virtual std::unique_ptr<Resource> createResource(ResourceCreateInfo& resourceCreateInfo) = 0;
	};
}

