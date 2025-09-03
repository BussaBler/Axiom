#pragma once
#include "VulkanRenderPass.h"
#include "Renderer/Core/Framebuffer.h"

namespace Axiom {
	class VulkanFramebuffer : public Framebuffer {
	public:
		VulkanFramebuffer(VulkanDevice& device, RenderPass& renderPass, std::vector<ImageView>& imageViews, uint32_t width, uint32_t height);
		~VulkanFramebuffer() override;

	private:
		VulkanDevice& device;
	};
}

