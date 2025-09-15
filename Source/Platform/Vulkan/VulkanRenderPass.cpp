#include "axpch.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImageView.h"

namespace Axiom {
	VulkanRenderPass::~VulkanRenderPass() {
		vkDeviceWaitIdle(device.getHandle());
		AX_CORE_LOG_INFO("Destroying Vulkan Render Pass...");
		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device.getHandle(), renderPass, nullptr);
			renderPass = VK_NULL_HANDLE;
		}
		destroyFramebuffers();
	}

	void VulkanRenderPass::init(Swapchain& swapchain) {
		AX_CORE_LOG_INFO("Initializing Vulkan Render Pass...");
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::array<VkAttachmentDescription, 2> attachments{};// TODO: Makes this configurable
		// Color attachment
		attachments[0].format = static_cast<VulkanSwapchain&>(swapchain).getImageFormat();
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		// Depth attachment
		attachments[1].format = device.getAdapter().getDepthFormat();
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentReference{};
		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachmentReference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		AX_CORE_ASSERT(vkCreateRenderPass(device.getHandle(), &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create Vulkan draw pass");
		createFramebuffers(static_cast<VulkanSwapchain&>(swapchain));
	}

	void VulkanRenderPass::begin(VulkanCommandBuffer& commandBuffer, VulkanRenderPassBeginInfo beginInfo, uint32_t framebufferIndex) const {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[framebufferIndex];
		renderPassInfo.renderArea.offset = { beginInfo.offset.x(), beginInfo.offset.y() };
		renderPassInfo.renderArea.extent = { beginInfo.extent.x(), beginInfo.extent.y() };

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { { createInfo.clearColor.x(), createInfo.clearColor.y(), createInfo.clearColor.z(), createInfo.clearColor.w() } };
		clearValues[1].depthStencil = { createInfo.clearDepth, createInfo.clearStencil };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer.getHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		commandBuffer.setState(CommandBufferState::IN_RENDER_PASS);
	}

	void VulkanRenderPass::end(VulkanCommandBuffer& commandBuffer) const {
		vkCmdEndRenderPass(commandBuffer.getHandle());
		commandBuffer.setState(CommandBufferState::RECORDING);
	}

	void VulkanRenderPass::recreateFramebuffers(Swapchain& swapchain) {
		destroyFramebuffers();
		createFramebuffers(static_cast<VulkanSwapchain&>(swapchain));
	}
	
	void VulkanRenderPass::createFramebuffers(VulkanSwapchain& swapchain) {
		framebuffers.resize(swapchain.getImageCount());
		for (uint32_t i = 0; i < swapchain.getImageCount(); i++) {
			VulkanImageView& imageView = static_cast<VulkanImageView&>(swapchain.getImage(i).getView({}));
			VulkanImageView& depthImageView = static_cast<VulkanImageView&>(swapchain.getDepthImage().getView({}));
			std::array<VkImageView, 2> attachments = {
				imageView.getHandle(),
				depthImageView.getHandle()
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapchain.getSwapchainCreateInfo().width;
			framebufferInfo.height = swapchain.getSwapchainCreateInfo().height;
			framebufferInfo.layers = 1;
			AX_CORE_ASSERT(vkCreateFramebuffer(device.getHandle(), &framebufferInfo, nullptr, &framebuffers[i]) == VK_SUCCESS, "Failed to create Vulkan framebuffer");
		}
	}

	void VulkanRenderPass::destroyFramebuffers() {
		for (auto framebuffer : framebuffers) {
			if (framebuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device.getHandle(), framebuffer, nullptr);
			}
		}
		framebuffers.clear();
	}
}
