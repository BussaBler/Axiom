#pragma once
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "Renderer/Core/RenderPass.h"

namespace Axiom {
	enum class VulkanRenderPassState {
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMITTED,
		NOT_ALLOCATED
	};

	class VulkanRenderPass : public RenderPass {
	public:
		VulkanRenderPass(VulkanDevice& vkDevice, SwapChain& vkSwapChain, Math::Vec2 offset, Math::Vec2 extent, Math::Vec4 clearColor, float depth, uint32_t stencil);
		~VulkanRenderPass() override;

		void begin(CommandBuffer& commandBuffer, Framebuffer& framebuffer) const override;
		void end(CommandBuffer& commandBuffer) const override;

		VulkanRenderPassState getState() const { return state; }

		void setExtent(Math::Vec2 newExtent) { extent = newExtent; }
		
	private:
		VulkanDevice& device;
		Math::Vec2 offset;
		Math::Vec2 extent;
		Math::Vec4 clearColor;
		float depth;
		uint32_t stencil;

		VulkanRenderPassState state;
	};
}

