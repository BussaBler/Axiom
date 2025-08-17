#pragma once
#include "VulkanRenderPass.h"

namespace Axiom {
	class VulkanFramebuffer {
	public:
		VulkanFramebuffer(VulkanDevice& device, VulkanRenderPass& renderPass, std::vector<VkImageView>& imageView, uint32_t width, uint32_t height);
		~VulkanFramebuffer();

		VkFramebuffer getHandle() const { return handle; }

	private:
		VulkanDevice& device;
		VulkanRenderPass& renderPass;
		VkFramebuffer handle = VK_NULL_HANDLE;
		std::vector<VkImageView> attachments;
	};
}

