#include "axpch.h"
#include "Framebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Axiom {
	std::unique_ptr<Framebuffer> Framebuffer::create(Device& device, RenderPass& renderPassRef, std::vector<ImageView>& imageViews, uint32_t width, uint32_t height) {
		return std::make_unique<VulkanFramebuffer>(static_cast<VulkanDevice&>(device), renderPassRef, imageViews, width, height);
	}
}
