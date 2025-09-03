#pragma once
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Renderer/Core/RendererContext.h"
#include "Renderer/Data/Geometry/Vertex.h"
#include "Shader/VulkanMaterialShader.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "VulkanSemaphore.h"
#include "VulkanTexture.h"

namespace Axiom {
	class VulkanContext : public RendererContext {
	public:
		VulkanContext() = default;
		virtual ~VulkanContext() = default;
		void init(Window* window) override;
		void shutdown() override;
		bool beginFrame(float deltaTime) override;
		void updateGlobalState(Math::Mat4 projection, Math::Mat4 view, Math::Vec3 viewPos, Math::Vec4 ambientColor, int mode) override;
		void updateObjectState(const GeometryRenderData& data) override;
		bool endFrame() override;
		void onResize(uint32_t width, uint32_t height) override;

	private:
		void createDevice();
		void createSwapChain();
		void recreateSwapChain();
		void createRenderPass();
		void createCommandBuffers();
		void createFramebuffer();
		void createSyncObjects();
		void createShaders();
		void createBuffers();
		void uploadData(CommandPool& pool, Fence* fence, Queue& queue, VulkanBuffer& buffer, void* data, uint64_t size, uint64_t offset = 0);

	private:
		VulkanDevice* vkDevice;
		std::unique_ptr<VulkanSwapChain> swapchain;
		std::unique_ptr<VulkanRenderPass> mainRenderPass;
		std::vector <std::unique_ptr<VulkanFramebuffer>> framebuffers;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> graphicsCommandBuffers;
		std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphores;
		std::vector<std::unique_ptr<Semaphore>> queueCompleteSemaphores;
		std::vector<std::unique_ptr<VulkanFence>> inFlightFences;
		std::vector<VulkanFence*> imagesInFlightFences;
		std::unique_ptr<VulkanBuffer> objectVertexBuffer;
		std::unique_ptr<VulkanBuffer> objectIndexBuffer;
		uint64_t geometryVertexOffset = 0;
		uint64_t geometryIndexOffset = 0;

		uint32_t inFlightFencesCount = 0;
		uint32_t currentFrame = 0;
		uint32_t imageIndex;
		bool recreatingSwapChain = false;
		uint64_t framebufferGen = 0;
		uint64_t lastFramebufferGen = 0;
		uint32_t framebufferWidth = 0;
		uint32_t framebufferHeight = 0;
		float frameDeltaTime = 0.0f;
	};
}

