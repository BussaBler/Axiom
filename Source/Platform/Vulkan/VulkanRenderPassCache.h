#pragma once
#include "Renderer/RenderPassCache.h"

namespace Axiom {
	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanRenderPass;

	class VulkanRenderPassCache : public RenderPassCache {
	public:
		VulkanRenderPassCache(VulkanDevice& vkDevice, VulkanSwapchain& vkSwapchain) : device(vkDevice), swapchain(vkSwapchain) {}
		~VulkanRenderPassCache() override;

		// Get or create (on miss) a render pass based on specific parameters
		RenderPassToken get(RenderPassCreateInfo renderPassCreateInfo) override;
		void updateSwapchain(Swapchain& swapchain) override;

	private:
		void begin(RenderPassToken& token, CommandBuffer& commandBuffer) override;
		void end(RenderPassToken& token, CommandBuffer& commandBuffer) override;

	private:
		VulkanDevice& device;
		VulkanSwapchain& swapchain;
		std::unordered_map<RenderPassCreateInfo, uint32_t> renderPasses;
		std::vector<std::unique_ptr<VulkanRenderPass>> renderPassPool;
	};
}

