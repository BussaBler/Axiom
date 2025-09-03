#include "axpch.h"
#include "VulkanImageView.h"

namespace Axiom {
	VulkanImageView::VulkanImageView(VulkanDevice& vkDevice, Image& vkImage, const ImageViewCreateInfo& createInfo) : device(vkDevice) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vkImage.getHandle<VkImage>();
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = createInfo.format;
		viewInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		handle.emplace<VkImageView>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateImageView(device.getHandle<VkDevice>(), &viewInfo, nullptr, std::any_cast<VkImageView>(&handle)) == VK_SUCCESS, "Failed to create Vulkan image view");
	}

	VulkanImageView::VulkanImageView(VulkanDevice& vkDevice, VkImage vkImage, const ImageViewCreateInfo& createInfo) : device(vkDevice) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vkImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = createInfo.format;
		viewInfo.subresourceRange.aspectMask = createInfo.aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		handle.emplace<VkImageView>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateImageView(device.getHandle<VkDevice>(), &viewInfo, nullptr, std::any_cast<VkImageView>(&handle)) == VK_SUCCESS, "Failed to create Vulkan image view");
	}

	VulkanImageView::~VulkanImageView() {
		if (handle.has_value()) {
			vkDestroyImageView(device.getHandle<VkDevice>(), getHandle<VkImageView>(), nullptr);
			handle.reset();
		}
	}
}
