#include "axpch.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "Math/AxMath.h"

namespace Axiom {
	VulkanSwapchain::VulkanSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, VulkanDevice& vkDevice, VulkanQueue& presentQueue)
		: Swapchain(swapchainCreateInfo), device(vkDevice), presentQueue(presentQueue), swapchain(VK_NULL_HANDLE), surface(VK_NULL_HANDLE),
			swapChainImageFormat(VK_FORMAT_UNDEFINED) {

	}

	VulkanSwapchain::~VulkanSwapchain() {
		AX_CORE_LOG_INFO("Destroying Vulkan Swapchain...");
		if (swapchain) {
			vkDestroySwapchainKHR(device.getHandle(), swapchain, nullptr);
			swapchain = VK_NULL_HANDLE;
		}
		if (surface) {
			vkDestroySurfaceKHR(device.getAdapter().getInstance().getHandle(), surface, nullptr);
			surface = VK_NULL_HANDLE;
		}

	}

	void VulkanSwapchain::build() {
		AX_CORE_LOG_INFO("Building Vulkan Swapchain...");
		VkSurfaceKHR newSurface = device.getAdapter().getInstance().createSurface(swapchainCreateInfo.windowHandle);
		auto surfaceFormats = device.getAdapter().getSurfaceFormats(newSurface);
		auto presentModes = device.getAdapter().getPresentModes(newSurface);
		uint32_t frameCount = swapchainCreateInfo.desiredFrameCount;
		VkSurfaceCapabilitiesKHR surfaceCapabilities = device.getAdapter().getSurfaceCapabilities(newSurface);
		VkImageUsageFlags supportedUsageFlags = surfaceCapabilities.supportedUsageFlags;

		VkSurfaceFormatKHR swapChainSurfaceFormat = surfaceFormats[0];
		for (const auto& format : surfaceFormats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				swapChainSurfaceFormat = format;
				break;
			}
		}
		swapChainImageFormat = swapChainSurfaceFormat.format;
		frameCount = Math::axClamp(frameCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		// TODO: add a config for this
		VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& presentMode : presentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				swapChainPresentMode = presentMode;
				break;
			}
			else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				swapChainPresentMode = presentMode;
			}
		}

		VkExtent2D swapChainExtent{swapchainCreateInfo.width, swapchainCreateInfo.height};
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
			swapChainExtent = surfaceCapabilities.currentExtent;
		}
		else {
			swapChainExtent.width = Math::axClamp(swapChainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			swapChainExtent.height = Math::axClamp(swapChainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.surface = newSurface;
		createInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
		createInfo.imageFormat = swapChainSurfaceFormat.format;
		createInfo.presentMode = swapChainPresentMode;
		createInfo.imageExtent = swapChainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = surfaceCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		createInfo.minImageCount = frameCount;
		createInfo.oldSwapchain = swapchain ? swapchain : VK_NULL_HANDLE;

		VkSwapchainKHR newSwapchain;
		VkResult result = vkCreateSwapchainKHR(device.getHandle(), &createInfo, nullptr, &newSwapchain);
		if (result != VK_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to create swapchain! VkResult: {0}", static_cast<int>(result));
			return;
		}

		if (swapchain) {
			vkDestroySwapchainKHR(device.getHandle(), swapchain, nullptr);
			vkDestroySurfaceKHR(device.getAdapter().getInstance().getHandle(), surface, nullptr);
		}
		
		swapchain = newSwapchain;
		surface = newSurface;

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device.getHandle(), swapchain, &imageCount, nullptr);
		frames.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getHandle(), swapchain, &imageCount, frames.data());

		ResourceViewCreateInfo resourceViewCreateInfo{};
		resourceViewCreateInfo.format = VulkanResource::getResourceFormat(swapChainSurfaceFormat.format);
		resourceViewCreateInfo.aspectMask = ResourceAspectMask::Color;

		frameImages.clear();
		frameImages.reserve(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			frameImages.push_back(std::make_unique<VulkanImage>(device, frames[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
			frameImages[i]->getView(resourceViewCreateInfo);
		}

		depthImage = std::make_unique<VulkanImage>(device, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED);
		ResourceCreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.width = swapchainCreateInfo.width;
		depthImageCreateInfo.height = swapchainCreateInfo.height;
		depthImageCreateInfo.format = VulkanResource::getResourceFormat(device.getAdapter().getDepthFormat());
		depthImageCreateInfo.usage = ResourceUsage::DepthStencil;
		depthImage->init(depthImageCreateInfo);

		ResourceViewCreateInfo depthResourceViewCreateInfo{};
		depthResourceViewCreateInfo.format = depthImageCreateInfo.format;
		depthResourceViewCreateInfo.aspectMask = ResourceAspectMask::Depth | ResourceAspectMask::Stencil;
		depthImage->getView(depthResourceViewCreateInfo);
	}

	void VulkanSwapchain::rebuild(const SwapchainCreateInfo& swapchainCreateInfo) {
		this->swapchainCreateInfo = swapchainCreateInfo;
		isRecreating = true;
		vkDeviceWaitIdle(device.getHandle());
		build();
		isRecreating = false;
	}

	void VulkanSwapchain::present(Context& context) {
		VulkanContext& vkContext = static_cast<VulkanContext&>(context);
		std::array<VkSwapchainKHR, 1> swapchains = { swapchain };
		std::array<VkSemaphore, 1> waitSemaphores = { vkContext.getCurrentRenderFinishedSemaphore() };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
		presentInfo.pSwapchains = swapchains.data();
		presentInfo.pImageIndices = &currentImageIndex;
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		presentInfo.pWaitSemaphores = waitSemaphores.data();

		VkResult result = vkQueuePresentKHR(presentQueue.getHandle(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			AX_CORE_LOG_WARN("Swapchain out of date during present, needs to be rebuilt");
			isRecreating = true;
		}
	}

	void VulkanSwapchain::prepare(Context& context) {
		VulkanContext& vkContext = static_cast<VulkanContext&>(context);
		AX_CORE_ASSERT(vkContext.getFrameCount() <= frameImages.size(), "Not enough swapchain images for the context frame count!");

		VulkanContextFrame currentFrame = vkContext.getCurrentFrameResource();
		VkResult result = vkAcquireNextImageKHR(device.getHandle(), swapchain, UINT64_MAX, currentFrame.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex);

		switch (result) {
			case VK_ERROR_OUT_OF_DATE_KHR:
				AX_CORE_LOG_WARN("Swapchain out of date during acquire, needs to be rebuilt");
				isRecreating = true;
				break;
			default:
				AX_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image!");
			break;
		}
	}
}
