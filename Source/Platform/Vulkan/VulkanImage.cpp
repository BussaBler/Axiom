#include "axpch.h"
#include "VulkanImage.h"

namespace Axiom {
	VulkanImage::VulkanImage(VulkanDevice& vkDevice, const ImageCreateInfo& createInfo)
		: device(vkDevice), width(createInfo.vkImageCreateInfo.extent.width), height(createInfo.vkImageCreateInfo.extent.height) {
		handle.emplace<VkImage>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateImage(device.getHandle<VkDevice>(), &createInfo.vkImageCreateInfo, nullptr, std::any_cast<VkImage>(&handle)) == VK_SUCCESS, "Failed to create Vulkan image");
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.getHandle<VkDevice>(), getHandle<VkImage>(), &memRequirements);
		
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, createInfo.memoryFlags);
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle<VkDevice>(), &allocInfo, nullptr, &imageMemory) == VK_SUCCESS, "Failed to allocate Vulkan image memory");
		
		vkBindImageMemory(device.getHandle<VkDevice>(), getHandle<VkImage>(), imageMemory, 0);
	}

	VulkanImage::~VulkanImage() {
		if (handle.has_value()) {
			vkDestroyImage(device.getHandle<VkDevice>(), getHandle<VkImage>(), nullptr);
			handle.reset();
		}
		if (imageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device.getHandle<VkDevice>(), imageMemory, nullptr);
			imageMemory = VK_NULL_HANDLE;
		}
	}

	void VulkanImage::transitionImageLayout(CommandBuffer& commandBuffer, Queue& queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		uint32_t graphicsFamily = device.findPhysicalQueueFamilies().graphicsFamily;
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = graphicsFamily;
		barrier.dstQueueFamilyIndex = graphicsFamily;
		barrier.image = getHandle<VkImage>();
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
		vkCmdPipelineBarrier(commandBuffer.getHandle<VkCommandBuffer>(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void VulkanImage::copyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer) const {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer.getHandle<VkCommandBuffer>(), buffer.getHandle<VkBuffer>(), getHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
}
