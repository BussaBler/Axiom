#include "axpch.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImageView.h"

namespace Axiom {
	VulkanRenderPass::~VulkanRenderPass() {
		device.waitIdle();
		AX_CORE_LOG_INFO("Destroying Vulkan Render Pass...");
		if (renderPass) {
			device.getHandle().destroyRenderPass(renderPass);
			renderPass = nullptr;
		}
		destroyFramebuffers();
	}

	void VulkanRenderPass::init(Swapchain& swapchain) {
		AX_CORE_LOG_INFO("Initializing Vulkan Render Pass...");

		std::array<Vk::AttachmentDescription, 2> attachments{};// TODO: Makes this configurable

		// Color attachment
		attachments[0].setFormat(static_cast<VulkanSwapchain&>(swapchain).getImageFormat());
		attachments[0].setSamples(Vk::SampleCountFlagBits::e1);
		attachments[0].setLoadOp(Vk::AttachmentLoadOp::eClear);
		attachments[0].setStoreOp(Vk::AttachmentStoreOp::eStore);
		attachments[0].setStencilLoadOp(Vk::AttachmentLoadOp::eDontCare);
		attachments[0].setStencilStoreOp(Vk::AttachmentStoreOp::eDontCare);
		attachments[0].setInitialLayout(Vk::ImageLayout::eUndefined);
		attachments[0].setFinalLayout(Vk::ImageLayout::ePresentSrcKHR);

		Vk::AttachmentReference colorAttachmentReference(0, Vk::ImageLayout::eColorAttachmentOptimal);

		// Depth attachment
		attachments[1].setFormat(device.getAdapter().getDepthFormat());
		attachments[1].setSamples(Vk::SampleCountFlagBits::e1);
		attachments[1].setLoadOp(Vk::AttachmentLoadOp::eClear);
		attachments[1].setStoreOp(Vk::AttachmentStoreOp::eDontCare);
		attachments[1].setStencilLoadOp(Vk::AttachmentLoadOp::eDontCare);
		attachments[1].setStencilStoreOp(Vk::AttachmentStoreOp::eDontCare);
		attachments[1].setInitialLayout(Vk::ImageLayout::eUndefined);
		attachments[1].setFinalLayout(Vk::ImageLayout::eDepthStencilAttachmentOptimal);

		Vk::AttachmentReference depthAttachmentReference(1, Vk::ImageLayout::eDepthStencilAttachmentOptimal);

		Vk::SubpassDescription subpassDescription({}, Vk::PipelineBindPoint::eGraphics, {}, colorAttachmentReference, {}, &depthAttachmentReference);
		Vk::SubpassDependency dependency(Vk::SubpassExternal, 0, Vk::PipelineStageFlagBits::eColorAttachmentOutput, Vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, Vk::AccessFlagBits::eColorAttachmentRead | Vk::AccessFlagBits::eColorAttachmentWrite);

		Vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpassDescription, dependency);
		Vk::ResultValue<Vk::RenderPass> renderPassResult = device.getHandle().createRenderPass(renderPassInfo);

		AX_CORE_ASSERT(renderPassResult.result == Vk::Result::eSuccess, "Failed to create Vulkan draw pass");
		renderPass = renderPassResult.value;
		createFramebuffers(static_cast<VulkanSwapchain&>(swapchain));
	}

	void VulkanRenderPass::begin(VulkanCommandBuffer& commandBuffer, VulkanRenderPassBeginInfo beginInfo, uint32_t framebufferIndex) const {
		Vk::Rect2D renderArea({ beginInfo.offset.x(), beginInfo.offset.y() }, { beginInfo.extent.x(), beginInfo.extent.y() });
		std::array<Vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor({ createInfo.clearColor.x(), createInfo.clearColor.y(), createInfo.clearColor.z(), createInfo.clearColor.w() });
		clearValues[1].setDepthStencil({ createInfo.clearDepth, createInfo.clearStencil });
		
		Vk::RenderPassBeginInfo renderPassInfo(renderPass, framebuffers[framebufferIndex], renderArea, clearValues);

		commandBuffer.getHandle().beginRenderPass(renderPassInfo, Vk::SubpassContents::eInline);
		commandBuffer.setState(CommandBufferState::IN_RENDER_PASS);
	}

	void VulkanRenderPass::end(VulkanCommandBuffer& commandBuffer) const {
		commandBuffer.getHandle().endRenderPass();
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
			std::array<Vk::ImageView, 2> attachments = {
				imageView.getHandle(),
				depthImageView.getHandle()
			};

			Vk::FramebufferCreateInfo framebufferInfo({}, renderPass, attachments, swapchain.getSwapchainCreateInfo().width, swapchain.getSwapchainCreateInfo().height, 1);
			Vk::ResultValue<Vk::Framebuffer> framebufferResult = device.getHandle().createFramebuffer(framebufferInfo);

			AX_CORE_ASSERT(framebufferResult.result == Vk::Result::eSuccess, "Failed to create Vulkan framebuffer");
			framebuffers[i] = framebufferResult.value;
		}
	}

	void VulkanRenderPass::destroyFramebuffers() {
		for (auto framebuffer : framebuffers) {
			if (framebuffer) {
				device.getHandle().destroyFramebuffer(framebuffer);	
			}
		}
		framebuffers.clear();
	}
}
