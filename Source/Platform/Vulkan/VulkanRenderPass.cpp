#include "axpch.h"
#include "VulkanRenderPass.h"

namespace Axiom {
	VulkanRenderPass::VulkanRenderPass(VulkanDevice& vkDevice, VulkanSwapChain* vkSwapChain, float x, float y, float w, float h, float r, float g, float b, float a, float depth, uint32_t stencil)
		: device(vkDevice), x(x), y(y), w(w), h(h), r(r), g(g), b(b), a(a), depth(depth), stencil(stencil), state(VulkanRenderPassState::READY) {
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		std::array<VkAttachmentDescription, 2> attachments{};
		// Color attachment
		attachments[0].format = vkSwapChain->getImageFormat().format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].flags = 0;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		// Depth attachment
		attachments[1].format = device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
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

		// Input attachments (input from shaders)
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;

		// Resolve attachments (for multisampling)
		subpass.pResolveAttachments = nullptr;

		// Preserve attachments
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;

		AX_CORE_ASSERT(vkCreateRenderPass(vkDevice.getHandle(), &renderPassInfo, nullptr, &handle) == VK_SUCCESS, "Failed to create Vulkan render pass");
	}

	VulkanRenderPass::~VulkanRenderPass() {
		vkDestroyRenderPass(device.getHandle(), handle, nullptr);
	}

	void VulkanRenderPass::begin(VulkanCommandBuffer commandBuffer, VkFramebuffer framebuffer) const {
		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass = handle;
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderArea.offset.x = x;
		beginInfo.renderArea.offset.y = y;
		beginInfo.renderArea.extent.width = w;
		beginInfo.renderArea.extent.height = h;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { r, g, b, a };
		clearValues[1].depthStencil = { depth, stencil };

		beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		beginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer.getHandle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		commandBuffer.setState(VulkanCommandBufferState::IN_RENDER_PASS);
	}

	void VulkanRenderPass::end(VulkanCommandBuffer commandBuffer) const {
		vkCmdEndRenderPass(commandBuffer.getHandle());
		commandBuffer.setState(VulkanCommandBufferState::RECORDING_ENDED);
	}
}
