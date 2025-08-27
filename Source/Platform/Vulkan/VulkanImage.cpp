#include "axpch.h"
#include "VulkanImage.h"

namespace Axiom {
	VulkanImage::VulkanImage(VulkanDevice& vkDevice, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspectFlags) 
		: device(vkDevice), width(createInfo.extent.width), height(createInfo.extent.height) {
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

	VulkanImage::~VulkanImage() {
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

	void VulkanImage::transitionImageLayout(VulkanCommandBuffer commandBuffer, VkQueue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		uint32_t graphicsFamily = device.findPhysicalQueueFamilies().graphicsFamily;
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = graphicsFamily;
		barrier.dstQueueFamilyIndex = graphicsFamily;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} 
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported layout transition!");
		}
		vkCmdPipelineBarrier(commandBuffer.getHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanImage::copyFromBuffer(VulkanCommandBuffer commandBuffer, VkBuffer buffer) const {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer.getHandle(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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
