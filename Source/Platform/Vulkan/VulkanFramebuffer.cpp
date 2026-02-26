#include "axpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"

namespace Axiom {
	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& vkDevice, VulkanRenderPass& vkRenderPass, const std::vector<Vk::ImageView>& attachments, uint32_t width, uint32_t height) 
		: device(vkDevice), renderPass(vkRenderPass), attachments(attachments) {
		Vk::FramebufferCreateInfo framebufferInfo({}, renderPass.getHandle(), attachments, width, height, 1);
		Vk::ResultValue<Vk::Framebuffer> framebufferResult = device.getHandle().createFramebuffer(framebufferInfo);

		AX_CORE_ASSERT(framebufferResult.result == Vk::Result::eSuccess, "Failed to create Vulkan Framebuffer!");
		framebuffer = framebufferResult.value;
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		if (framebuffer != VK_NULL_HANDLE) {
			device.getHandle().destroyFramebuffer(framebuffer);
			framebuffer = VK_NULL_HANDLE;
		}
	}
}
