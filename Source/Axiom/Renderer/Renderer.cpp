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
		RenderPassCreateInfo renderPassInfo{};
		renderPassInfo.clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
		auto mainRender = renderPassCache->get(renderPassInfo);

		shader = device->createShader(mainRender);
		ResourceCreateInfo vertexBufferInfo{};
		vertexBufferInfo.size = 3 * sizeof(float) * 4;
		vertexBufferInfo.usage = ResourceUsage::VertexBuffer | ResourceUsage::TransferDst;
		vertexBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		vertexBuffer = device->createResource(vertexBufferInfo);
		std::array<Math::Vec3, 3> vertices = {
			Math::Vec3(0.0f, 0.5f, 0.0f),
			Math::Vec3(-0.5f, -0.5f, 0.0f),
			Math::Vec3(0.5f, -0.5f, 0.0f)
		};
		vertexBuffer->loadData(vertices.data(), vertexBufferInfo.size);
		ResourceCreateInfo indexBufferInfo{};
		indexBufferInfo.size = 3 * sizeof(uint32_t);
		indexBufferInfo.usage = ResourceUsage::IndexBuffer | ResourceUsage::TransferDst;
		indexBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		indexBuffer = device->createResource(indexBufferInfo);
		std::array<uint32_t, 3> indices = { 0, 1, 2 };
		indexBuffer->loadData(indices.data(), indexBufferInfo.size);
		ResourceCreateInfo uniformBufferInfo{};
		uniformBufferInfo.size = sizeof(GlobalUbo) * 3;
		uniformBufferInfo.usage = ResourceUsage::UniformBuffer | ResourceUsage::TransferDst | ResourceUsage::TransferSrc;
		uniformBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		uniformBuffer = device->createResource(uniformBufferInfo);
		globalUbo.view = Math::Mat4::identity();
		globalUbo.proj = Math::Mat4::identity();
		globalUbo.color = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		shader->bindUniformBuffer(*uniformBuffer);
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
			
			shader->bind(context->getMainCommandBuffer());
			bindVertexBuffer(*vertexBuffer);
			bindIndexBuffer(*indexBuffer);
			uniformBuffer->loadData(&globalUbo, sizeof(GlobalUbo), sizeof(GlobalUbo) * context->getCurrentFrameIndex());
			shader->bindDescriptors(context->getMainCommandBuffer(), context->getCurrentFrameIndex() * sizeof(GlobalUbo));
			
			context->drawIndexed(3, 1, context->getMainCommandBuffer());
			mainRender.end(context->getMainCommandBuffer());
			context->end(*swapchain);
			swapchain->present(*context);
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

	void Renderer::bindVertexBuffer(Resource& vertexBuffer) {
		context->bindVertexBuffer(vertexBuffer, context->getMainCommandBuffer());
	}

	void Renderer::bindIndexBuffer(Resource& indedxBuffer) {
		context->bindIndexBuffer(indedxBuffer, context->getMainCommandBuffer());
	}
}
