#include "axpch.h"
#include "Renderer.h"
#include "Instance.h"
#include "Device.h"
#include "RenderPassCache.h"
#include "Texture.h"
#include "Vertex.h"
#include "Core/Input.h"

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
		renderPassInfo.clearColor = { 0.2f, 0.2f, 1.0f, 1.0f };
		auto mainRender = renderPassCache->get(renderPassInfo);

		shader = device->createShader(mainRender);

		ResourceCreateInfo stagingBufferInfo{};
		stagingBufferInfo.size = 4 * sizeof(Vertex);
		stagingBufferInfo.usage = ResourceUsage::TransferSrc;
		stagingBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		auto stagingBuffer = device->createResource(stagingBufferInfo);
		std::array<Vertex, 4> vertices = {
			Vertex(Math::Vec3(-0.5f, 0.5f, 0.0f), Math::Vec2(0.0f, 1.0f)),
			Vertex(Math::Vec3(-0.5f, -0.5f, 0.0f), Math::Vec2(0.0f, 0.0f)),
			Vertex(Math::Vec3(0.5f, -0.5f, 0.0f), Math::Vec2(1.0f, 0.0f)),
			Vertex(Math::Vec3(0.5f, 0.5f, 0.0f), Math::Vec2(1.0f, 1.0f))
		};
		stagingBuffer->loadData(vertices.data(), stagingBufferInfo.size);

		ResourceCreateInfo vertexBufferInfo{};
		vertexBufferInfo.size = 4 * sizeof(Vertex);
		vertexBufferInfo.usage = ResourceUsage::VertexBuffer | ResourceUsage::TransferDst;
		vertexBufferInfo.memoryUsage = ResourceMemoryUsage::GPU_Only;
		vertexBuffer = device->createResource(vertexBufferInfo);
		stagingBuffer->copyTo(*vertexBuffer);

		ResourceCreateInfo indexBufferInfo{};
		indexBufferInfo.size = 6 * sizeof(uint32_t);
		indexBufferInfo.usage = ResourceUsage::IndexBuffer | ResourceUsage::TransferDst;
		indexBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		indexBuffer = device->createResource(indexBufferInfo);
		std::array<uint32_t, 6> indices = { 0, 1, 2, 2, 3, 0 };
		indexBuffer->loadData(indices.data(), indexBufferInfo.size);

		ResourceCreateInfo uniformBufferInfo{};
		uniformBufferInfo.size = sizeof(GlobalUbo) + sizeof(MaterialUbo);
		uniformBufferInfo.usage = ResourceUsage::UniformBuffer | ResourceUsage::TransferDst | ResourceUsage::TransferSrc;
		uniformBufferInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		uniformBuffer = device->createResource(uniformBufferInfo);
		globalUbo.view = Math::Mat4::translate(Math::Vec3(0.0f, 0.0f, -2.0f));
		globalUbo.proj = Math::Mat4::perspective(Math::toRadians(45.0f), 1280.0f / 720.0f, 0.1f, 1000.0f);
		materialUbo.diffuseColor = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);

		createDefaultResources();
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
			uniformBuffer->loadData(&globalUbo, sizeof(GlobalUbo));
			uniformBuffer->loadData(&materialUbo, sizeof(MaterialUbo), sizeof(GlobalUbo));
			shader->updateFrameIndex(context->getCurrentFrameIndex());
			shader->bindUniformBuffer(*uniformBuffer);
			shader->bindTexture(*defaultTexture);
			shader->bindPushConstants(context->getMainCommandBuffer(), &rotation, sizeof(Math::Mat4));
			shader->bindDescriptors(context->getMainCommandBuffer());

			context->drawIndexed(6, 1, context->getMainCommandBuffer());
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

	void Renderer::bindIndexBuffer(Resource& indexBuffer) {
		context->bindIndexBuffer(indexBuffer, context->getMainCommandBuffer());
	}

	void Renderer::createDefaultResources() {
		TextureCreateInfo textureCreateInfo{};
		textureCreateInfo.width = 256;
		textureCreateInfo.height = 256;
		textureCreateInfo.channels = 4;

		std::vector<uint8_t> pixels(256 * 256 * 4);
		for (uint32_t y = 0; y < 256; ++y) {
			for (uint32_t x = 0; x < 256; ++x) {
				const bool purple = ((x / 32) + (y / 32)) % 2 == 0;
				const size_t base = (static_cast<size_t>(y) * 256 + x) * 4;
				const uint8_t v = purple ? 255 : 0;
				pixels[base + 0] = v;
				pixels[base + 1] = 0;
				pixels[base + 2] = v;
				pixels[base + 3] = 255;
			}
		}

		textureCreateInfo.data = pixels.data();
		defaultTexture = device->createTexture(textureCreateInfo);
	}
}
