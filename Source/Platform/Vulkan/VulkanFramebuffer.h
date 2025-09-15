#pragma once
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanRenderPass;

	class VulkanFramebuffer {
	public:
		VulkanFramebuffer(VulkanDevice& vkDevice, VulkanRenderPass& vkRenderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height);
		~VulkanFramebuffer();

	private:
		VulkanDevice& device;
		VulkanRenderPass& renderPass;
		VkFramebuffer framebuffer;
		std::vector<VkImageView> attachments;
	};
}

