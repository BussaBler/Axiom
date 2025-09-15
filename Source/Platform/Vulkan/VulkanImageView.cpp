#include "axpch.h"
#include "VulkanImageView.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanImageView::~VulkanImageView() {
		if (imageView) {
			vkDestroyImageView(device.getHandle(), imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}
	}

	void VulkanImageView::init(const ResourceViewCreateInfo& resourceCreateInfo) {
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = image.getHandle();
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: make this configurable
		viewCreateInfo.format = VulkanResource::getVkFormat(resourceCreateInfo.format);
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange.aspectMask = getVkImageAspectFlags(resourceCreateInfo.aspectMask);
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		AX_CORE_ASSERT(vkCreateImageView(device.getHandle(), &viewCreateInfo, nullptr, &imageView) == VK_SUCCESS, "Failed to create image view!");
	}

	VkImageAspectFlags VulkanImageView::getVkImageAspectFlags(uint32_t format) {
		VkImageAspectFlags flags{};
		if (format & ResourceAspectMask::Color) {
			flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (format & ResourceAspectMask::Depth) {
			flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (format & ResourceAspectMask::Stencil) {
			flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		return flags;
	}
}
