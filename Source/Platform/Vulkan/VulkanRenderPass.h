#pragma once
#include "Renderer/RenderPassCache.h"
#include "Renderer/Swapchain.h"
#include "Math/AxMath.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanSwapchain;
	class VulkanDevice;
	class VulkanCommandBuffer;

	enum class RenderPassState {
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMITTED,
		NOT_ALLOCATED
	};

	struct VulkanRenderPassBeginInfo {
		Math::uVec2 extent;
		Math::iVec2 offset = { 0, 0 };
	};

	class VulkanRenderPass {
	public:
		VulkanRenderPass(VulkanDevice& vkDevice, RenderPassCreateInfo rendePassCreateInfo) :
			device(vkDevice), renderPass(VK_NULL_HANDLE), createInfo(rendePassCreateInfo) {}

		~VulkanRenderPass();

		void init(Swapchain& swapchain);
		void begin(VulkanCommandBuffer& commandBuffer, VulkanRenderPassBeginInfo beginInfo, uint32_t framebufferIndex) const;
		void end(VulkanCommandBuffer& commandBuffer) const;
		void recreateFramebuffers(Swapchain& swapchain);

		VkRenderPass getHandle() const { return renderPass; }

	private:
		void createFramebuffers(VulkanSwapchain& swapchain);
		void destroyFramebuffers();

	private:
		VulkanDevice& device;
		VkRenderPass renderPass;
		RenderPassCreateInfo createInfo;
		RenderPassState state = RenderPassState::NOT_ALLOCATED;
		std::vector<VkFramebuffer> framebuffers;
	};
}

