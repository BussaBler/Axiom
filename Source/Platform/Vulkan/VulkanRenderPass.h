#pragma once
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"

namespace Axiom {
	enum class VulkanRenderPassState {
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMITTED,
		NOT_ALLOCATED
	};

	class VulkanRenderPass {
	public:
		VulkanRenderPass(VulkanDevice& vkDevice, VulkanSwapChain* vkSwapChain, float x, float y, float w, float h, float r, float g, float b, float a, float depth, uint32_t stencil);
		~VulkanRenderPass();

		void begin(VulkanCommandBuffer commandBuffer, VkFramebuffer framebuffer) const;
		void end(VulkanCommandBuffer commandBuffer) const;

		VkRenderPass getHandle() const { return handle; }
		VulkanRenderPassState getState() const { return state; }
		void setWidth(float width) { w = width; }
		void setHeight(float height) { h = height; }
		
	private:
		VulkanDevice& device;
		VkRenderPass handle;
		float x, y, w, h;
		float r, g, b, a;
		float depth;
		uint32_t stencil;

		VulkanRenderPassState state;
	};
}

