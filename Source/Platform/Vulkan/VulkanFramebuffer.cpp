#include "axpch.h"
#include "VulkanFramebuffer.h"

namespace Axiom {
	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, VulkanRenderPass& renderPass, std::vector<VkImageView>& imageView, uint32_t width, uint32_t height) 
		: device(device), renderPass(renderPass), attachments(imageView) {
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass.getHandle();
		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;

		AX_CORE_ASSERT(vkCreateFramebuffer(device.getHandle(), &createInfo, nullptr, &handle) == VK_SUCCESS, "Failed to create vulkan framebuffer");
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		vkDestroyFramebuffer(device.getHandle(), handle, nullptr);
	}
}