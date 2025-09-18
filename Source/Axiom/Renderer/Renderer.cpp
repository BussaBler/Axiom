#include "axpch.h"
#include "Renderer.h"
#include "Instance.h"
#include "Device.h"
#include "RenderPassCache.h"

namespace Axiom {
	Renderer::Renderer() = default;
	Renderer::~Renderer() = default;

	void Renderer::init(void* windowHandle) {
		InstaceInfo info{};
		info.appName = "Theorem";
		info.appVersionMajor = 0;
		info.appVersionMinor = 1;
		info.appVersionPatch = 0;
		info.engineName = "Axiom";
		info.engineVersionMajor = 0;
		info.engineVersionMinor = 1;
		info.engineVersionPatch = 0;

		instance = Instance::create(GraphicsAPI::VULKAN);
		instance->init(info);

		adapter = instance->getAdapter();
		device = adapter->createDevice();
		device->init({});
		context = device->createContext();
		SwapchainCreateInfo swapchainCreateInfo{};
		swapchainCreateInfo.width = 1280;
		swapchainCreateInfo.height = 720;
		swapchainCreateInfo.windowHandle = windowHandle;
		swapchain = device->createSwapchain(swapchainCreateInfo);
		renderPassCache = device->createRenderPassCache(*swapchain);
	}

	void Renderer::shutdown() {
		// TODO: implement
	}

	void Renderer::draw() {
		if (context->begin(*swapchain)) {
			RenderPassCreateInfo renderPassInfo{};
			renderPassInfo.clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
			auto mainRender = renderPassCache->get(renderPassInfo);
			mainRender.begin(context->getMainCommandBuffer());
			mainRender.end(context->getMainCommandBuffer());
			context->end(*swapchain);
			swapchain->present(*context);
			context->incrementFrameIndex();
		}
		
	}

	void Renderer::resize(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) {
			return;
		}
		SwapchainCreateInfo swapchainCreateInfo = swapchain->getSwapchainCreateInfo();
		swapchainCreateInfo.width = width;
		swapchainCreateInfo.height = height;
		swapchain->rebuild(swapchainCreateInfo);
		renderPassCache->updateSwapchain(*swapchain);
	}
}
