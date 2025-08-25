#pragma once
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Renderer/RendererContext.h"
#include "Shader/VulkanObjectShader.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"

namespace Axiom {
	class VulkanContext : public RendererContext {
	public:
		VulkanContext() = default;
		virtual ~VulkanContext() = default;
		void init(Window* window) override;
		void shutdown() override;
		bool beginFrame() override;
		void updateGlobalState(Math::Mat4 projection, Math::Mat4 view, Math::Vec3 viewPos, Math::Vec4 ambientColor, int mode) override;
		void updateObjectState(Math::Mat4 model) override;
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
		void uploadData(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer& buffer, void* data, uint64_t size, uint64_t offset = 0);

	private:
		std::unique_ptr<VulkanDevice> device;
		std::unique_ptr<VulkanSwapChain> swapchain;
		std::unique_ptr<VulkanRenderPass> mainRenderPass;
		std::vector <std::unique_ptr<VulkanFramebuffer>> framebuffers;
		std::vector<std::unique_ptr<VulkanCommandBuffer>> graphicsCommandBuffers;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> queueCompleteSemaphores;
		std::vector<std::unique_ptr<VulkanFence>> inFlightFences;
		std::vector<VulkanFence*> imagesInFlightFences;
		std::unique_ptr<VulkanObjectShader> objectShader;
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
	};
}

