#include "axpch.h"
#include "VulkanFramebuffer.h"

namespace Axiom {
	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, RenderPass& renderPass, std::vector<ImageView>& imageViews, uint32_t width, uint32_t height)
		: device(device) {
		std::vector<VkImageView> attachments;
		attachments.reserve(imageViews.size());
		for (const auto& imageView : imageViews) {
			attachments.push_back(imageView.getHandle<VkImageView>());
		}
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass.getHandle<VkRenderPass>();
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;
		handle.emplace<VkFramebuffer>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateFramebuffer(device.getHandle<VkDevice>(), &createInfo, nullptr, std::any_cast<VkFramebuffer>(&handle)) == VK_SUCCESS, "Failed to create vulkan framebuffer");
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		vkDestroyFramebuffer(device.getHandle<VkDevice>(), getHandle<VkFramebuffer>(), nullptr);
	}
}