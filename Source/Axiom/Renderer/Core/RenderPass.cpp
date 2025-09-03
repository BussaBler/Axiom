#include "axpch.h"
#include "RenderPass.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

namespace Axiom {
	std::unique_ptr<RenderPass> RenderPass::create(Device& deviceRef, SwapChain& swapChainRef, Math::Vec2 offset, Math::Vec2 extent, Math::Vec4 clearColor, float depth, uint32_t stencil) {
		return std::make_unique<VulkanRenderPass>(static_cast<VulkanDevice&>(deviceRef), swapChainRef, offset, extent, clearColor, depth, stencil);
	}
}
