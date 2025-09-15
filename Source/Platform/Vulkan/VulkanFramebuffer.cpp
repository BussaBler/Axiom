#include "axpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

namespace Axiom {
	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& vkDevice, VulkanRenderPass& vkRenderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height) 
		: device(vkDevice), renderPass(vkRenderPass), attachments(attachments) {
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass.getHandle();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;
		AX_CORE_ASSERT(vkCreateFramebuffer(device.getHandle(), &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS, "Failed to create Vulkan Framebuffer!");
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		if (framebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(device.getHandle(), framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}
	}
}
