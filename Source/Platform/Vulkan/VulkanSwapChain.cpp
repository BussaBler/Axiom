#include "axpch.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanSwapChain::VulkanSwapChain(VulkanDevice& device, VkExtent2D extent) : device(device) {
		createSwapChain(extent);
	}

	VulkanSwapChain::~VulkanSwapChain() {
		destroySwapChain();
	}

	void VulkanSwapChain::recreate(VkExtent2D extent) {
		if (extent.width == 0 || extent.height == 0) {
			AX_CORE_LOG_WARN("Attempted to recreate swap chain with zero extent");
			return;
		}
		destroySwapChain();
		createSwapChain(extent);
	}

	void VulkanSwapChain::acquireNextImageIndex(uint32_t& imageIndex, VkSemaphore semaphore, VkFence fence, VkExtent2D extent) {
		VkResult result = vkAcquireNextImageKHR(device.getHandle(), handle, UINT64_MAX, semaphore, fence, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate(extent);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			AX_CORE_LOG_ERROR("Failed to acquire next image index: {0}", static_cast<int>(result));
		}
	}

	void VulkanSwapChain::presentImage(VkQueue graphicsQueue, VkQueue presentQueue, VkSemaphore renderSemaphore, uint32_t imageIndex, uint32_t& currentFrame, VkExtent2D extent) {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &handle;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreate(extent);
		} else if (result != VK_SUCCESS) {
			AX_CORE_LOG_ERROR("Failed to present image: {0}", static_cast<int>(result));
		}
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanSwapChain::createSwapChain(VkExtent2D extent) {
		const auto supports = device.getSwapChainSupport();
		for (const auto& format : supports.formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				swapChainImageFormat = format;
				break;
			}
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& mode : supports.presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = mode;
				break;
			}
		}

		uint32_t imageCount = supports.capabilities.minImageCount + 1;
		if (supports.capabilities.maxImageCount > 0 && imageCount > supports.capabilities.maxImageCount) {
			imageCount = supports.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = device.getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = swapChainImageFormat.format;
		createInfo.imageColorSpace = swapChainImageFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto queueFamilyIndices = device.findPhysicalQueueFamilies();
		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
			uint32_t queueFamilyIndicesArray[] = { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = supports.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		AX_CORE_ASSERT(vkCreateSwapchainKHR(device.getHandle(), &createInfo, nullptr, &handle) == VK_SUCCESS, "Failed to create swap chain");
	
		uint32_t swapChainImagesCount = 0;
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &swapChainImagesCount, nullptr);
		swapChainImages.resize(swapChainImagesCount);
		swapChainImageViews.resize(swapChainImagesCount);
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &swapChainImagesCount, swapChainImages.data());
		for (uint32_t i = 0; i < swapChainImagesCount; i++) {
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = swapChainImageFormat.format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			AX_CORE_ASSERT(vkCreateImageView(device.getHandle(), &viewInfo, nullptr, &swapChainImageViews[i]) == VK_SUCCESS, "Failed to create image views");
		}

		VkFormat depthFormat = device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		VkImageCreateInfo depthImageInfo = {};
		depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImageInfo.extent.width = extent.width;
		depthImageInfo.extent.height = extent.height;
		depthImageInfo.extent.depth = 1;
		depthImageInfo.mipLevels = 1;
		depthImageInfo.arrayLayers = 1;
		depthImageInfo.format = depthFormat;
		depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depthImageInfo.flags = 0;

        depthImage.create(device, depthImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void VulkanSwapChain::destroySwapChain() {
		depthImage.destroy(device);
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device.getHandle(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(device.getHandle(), handle, nullptr);
	}
}
