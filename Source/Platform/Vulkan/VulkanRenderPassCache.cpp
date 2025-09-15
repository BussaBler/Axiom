#include "axpch.h"
#include "VulkanRenderPassCache.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanRenderPassCache::~VulkanRenderPassCache() {
		renderPasses.clear();
	}

	RenderPassToken VulkanRenderPassCache::get(RenderPassCreateInfo renderPassCreateInfo) {
		auto it = renderPasses.find(renderPassCreateInfo);
		if (it != renderPasses.end()) {
			return RenderPassToken(it->second, 0, this);
		}

		renderPassPool.emplace_back(std::make_unique<VulkanRenderPass>(device, renderPassCreateInfo));
		uint32_t newIndex = static_cast<uint32_t>(renderPassPool.size() - 1);
		renderPassPool[newIndex]->init(swapchain);
		renderPasses[renderPassCreateInfo] = newIndex;
		return RenderPassToken(newIndex, 0, this);
	}

	void VulkanRenderPassCache::updateSwapchain(Swapchain& swapchain) {
		for (const auto& renderPass : renderPassPool) {
			renderPass->recreateFramebuffers(swapchain);
		}
	}

	void VulkanRenderPassCache::begin(RenderPassToken& token, CommandBuffer& commandBuffer) {
		VulkanRenderPassBeginInfo beginInfo{};
		beginInfo.extent = { swapchain.getSwapchainCreateInfo().width, swapchain.getSwapchainCreateInfo().height };
		renderPassPool[token.getIndex()]->begin(static_cast<VulkanCommandBuffer&>(commandBuffer), beginInfo, swapchain.getCurrentImageIndex());
	}

	void VulkanRenderPassCache::end(RenderPassToken& token, CommandBuffer& commandBuffer) {
		renderPassPool[token.getIndex()]->end(static_cast<VulkanCommandBuffer&>(commandBuffer));
	}


}
