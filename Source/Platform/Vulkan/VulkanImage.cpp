#include "axpch.h"
#include "VulkanImage.h"

namespace Axiom {
	void VulkanImage::create(VulkanDevice& device, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspectFlags) {
		AX_CORE_ASSERT(vkCreateImage(device.getHandle(), &createInfo, nullptr, &image) == VK_SUCCESS, "Failed to create Vulkan image");
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getHandle(), image, &memRequirements);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryFlags);
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle(), &allocInfo, nullptr, &imageMemory) == VK_SUCCESS, "Failed to allocate Vulkan image memory");
		vkBindImageMemory(device.getHandle(), image, imageMemory, 0);
		createImageView(device, createInfo.format, aspectFlags);
	}

	void VulkanImage::destroy(VulkanDevice& device) {
		if (imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(device.getHandle(), imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}
		if (image != VK_NULL_HANDLE) {
			vkDestroyImage(device.getHandle(), image, nullptr);
			image = VK_NULL_HANDLE;
		}
		if (imageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device.getHandle(), imageMemory, nullptr);
			imageMemory = VK_NULL_HANDLE;
		}
	}

	void VulkanImage::createImageView(VulkanDevice& device, VkFormat format, VkImageAspectFlagBits aspectFlags) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		AX_CORE_ASSERT(vkCreateImageView(device.getHandle(), &viewInfo, nullptr, &imageView) == VK_SUCCESS, "Failed to create Vulkan image view");
	}
}
