#pragma once
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;
	class VulkanRenderPass;

	class VulkanFramebuffer {
	public:
		VulkanFramebuffer(VulkanDevice& vkDevice, VulkanRenderPass& vkRenderPass, const std::vector<Vk::ImageView>& attachments, uint32_t width, uint32_t height);
		~VulkanFramebuffer();

	private:
		VulkanDevice& device;
		VulkanRenderPass& renderPass;
		Vk::Framebuffer framebuffer;
		std::vector<Vk::ImageView> attachments;
	};
}

