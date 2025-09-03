#include "axpch.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanSwapChain::VulkanSwapChain(VulkanDevice& device, Math::uVec2 extent) : device(device) {
		createSwapChain({ extent.x(), extent.y() });
	}

	VulkanSwapChain::~VulkanSwapChain() {
		destroySwapChain();
	}

	void VulkanSwapChain::recreate(Math::uVec2 extent) {
		if (extent.x() == 0 || extent.y() == 0) {
			AX_CORE_LOG_WARN("Attempted to recreate swap chain with zero extent");
			return;
		}
		destroySwapChain();
		createSwapChain({ extent.x(), extent.y() });
	}

	void VulkanSwapChain::acquireNextImageIndex(uint32_t& imageIndex, const Semaphore& semaphore, Fence* fence, Math::uVec2 extent) {
		VkResult result;
		if (fence) {
			result = vkAcquireNextImageKHR(device.getHandle<VkDevice>(), getHandle<VkSwapchainKHR>(), UINT64_MAX, semaphore.getHandle<VkSemaphore>(), fence->getHandle<VkFence>(), &imageIndex);
		}
		else {
			result = vkAcquireNextImageKHR(device.getHandle<VkDevice>(), getHandle<VkSwapchainKHR>(), UINT64_MAX, semaphore.getHandle<VkSemaphore>(), VK_NULL_HANDLE, &imageIndex);
		}
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate(extent);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			AX_CORE_LOG_ERROR("Failed to acquire next image index: {0}", static_cast<int>(result));
		}
	}

	void VulkanSwapChain::presentImage(const Queue& graphicsQueue, const Queue& presentQueue, const Semaphore& renderSemaphore, uint32_t imageIndex, uint32_t& currentFrame, Math::uVec2 extent) {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderSemaphore.getHandlePtr<VkSemaphore>();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = std::any_cast<VkSwapchainKHR>(&handle);
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(presentQueue.getHandle<VkQueue>(), &presentInfo);
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

		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = device.getSurface();
		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = swapChainImageFormat.format;
		swapChainCreateInfo.imageColorSpace = swapChainImageFormat.colorSpace;
		swapChainCreateInfo.imageExtent = extent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto queueFamilyIndices = device.findPhysicalQueueFamilies();
		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
			uint32_t queueFamilyIndicesArray[] = { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily };
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
		} else {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0;
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		swapChainCreateInfo.preTransform = supports.capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		swapChainCreateInfo.pNext = nullptr;

		handle.emplace<VkSwapchainKHR>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateSwapchainKHR(device.getHandle<VkDevice>(), &swapChainCreateInfo, nullptr, std::any_cast<VkSwapchainKHR>(&handle)) == VK_SUCCESS, "Failed to create swap chain");

		uint32_t swapChainImagesCount = 0;
		vkGetSwapchainImagesKHR(device.getHandle<VkDevice>(), getHandle<VkSwapchainKHR>(), &swapChainImagesCount, nullptr);
		swapChainImages.resize(swapChainImagesCount);
		swapChainImageViews.resize(swapChainImagesCount);
		vkGetSwapchainImagesKHR(device.getHandle<VkDevice>(), getHandle<VkSwapchainKHR>(), &swapChainImagesCount, swapChainImages.data());
		for (uint32_t i = 0; i < swapChainImagesCount; i++) {
			VkImageViewUsageCreateInfo usageInfo{};
			usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
			usageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			usageInfo.pNext = nullptr;
			ImageViewCreateInfo viewInfo{};
			viewInfo.format = swapChainImageFormat.format;
			viewInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
			swapChainImageViews[i] = std::make_unique<VulkanImageView>(device, swapChainImages[i], viewInfo);
		}

		VkFormat depthFormat = device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		ImageCreateInfo depthImageInfo = {};
		depthImageInfo.vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageInfo.vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImageInfo.vkImageCreateInfo.extent.width = extent.width;
		depthImageInfo.vkImageCreateInfo.extent.height = extent.height;
		depthImageInfo.vkImageCreateInfo.extent.depth = 1;
		depthImageInfo.vkImageCreateInfo.mipLevels = 1;
		depthImageInfo.vkImageCreateInfo.arrayLayers = 1;
		depthImageInfo.vkImageCreateInfo.format = depthFormat;
		depthImageInfo.vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageInfo.vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImageInfo.vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageInfo.vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		depthImageInfo.vkImageCreateInfo.flags = 0;
		depthImageInfo.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		depthImageInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

        depthImage = std::make_unique<VulkanImage>(device, depthImageInfo);
		ImageViewCreateInfo depthViewInfo{};
		depthViewInfo.format = depthFormat;
		depthViewInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthImageView = std::make_unique<VulkanImageView>(device, *depthImage, depthViewInfo);
	}

	void VulkanSwapChain::destroySwapChain() {
		depthImage.reset();
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			swapChainImageViews[i].reset();	
		}
		vkDestroySwapchainKHR(device.getHandle<VkDevice>(), getHandle<VkSwapchainKHR>(), nullptr);
	}
}
