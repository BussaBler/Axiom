#include "axpch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"

namespace Axiom {
	VulkanImage::VulkanImage(VulkanDevice& vkDevice, VkImage vkImage, VkImageLayout vkCurrentLayout)
		: VulkanResource(vkDevice), image(vkImage), currentLayout(vkCurrentLayout), format(VK_FORMAT_UNDEFINED) {}

	VulkanImage::~VulkanImage() {
		if (image && shouldClean) {
			vkDestroyImage(device.getHandle(), image, nullptr);
			image = VK_NULL_HANDLE;
		}
	}

	void VulkanImage::loadData(void* data, uint64_t size, uint64_t offset, uint32_t flags) {
		VulkanBuffer stagingBuffer(device);
		ResourceCreateInfo stagingBufferCreateInfo;
		stagingBufferCreateInfo.size = size;
		stagingBufferCreateInfo.usage = ResourceUsage::TransferSrc;
		stagingBufferCreateInfo.type = ResourceType::Buffer;
		stagingBufferCreateInfo.memoryUsage = ResourceMemoryUsage::CPU_To_GPU;
		stagingBuffer.init(stagingBufferCreateInfo);
		stagingBuffer.loadData(data, size, offset);

		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocateAndBeginSingleUse(device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getCommandPool());
		transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyFromBuffer(commandBuffer, stagingBuffer.getHandle());
		transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		commandBuffer.endSingleUse(device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getHandle(), device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getCommandPool());
	}

	ResourceView& VulkanImage::getView(const ResourceViewCreateInfo& resourceViewCreateInfo) {
		if (resourceView) {
			return *resourceView;
		}
		std::unique_ptr<VulkanImageView> imageView = std::make_unique<VulkanImageView>(device, *this);
		imageView->init(resourceViewCreateInfo);
		resourceView = std::move(imageView);
		return *resourceView;
	}

	void VulkanImage::init(const ResourceCreateInfo& resourceCreateInfo) {
		createInfo = resourceCreateInfo;
		format = getVkFormat(resourceCreateInfo.format);
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.extent = { resourceCreateInfo.width, resourceCreateInfo.height, 1 };
		imageCreateInfo.format = format;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // TODO: make this configurable
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.mipLevels = 1; // TODO: make this configurable
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: make this configurable
		imageCreateInfo.usage = getVkImageUsageFlags(resourceCreateInfo.usage);
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		AX_CORE_ASSERT(vkCreateImage(device.getHandle(), &imageCreateInfo, nullptr, &image) == VK_SUCCESS, "Failed to create image!");

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device.getHandle(), image, &memoryRequirements);
		memorySize = memoryRequirements.size;
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = device.getAdapter().findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // TODO: make this configurable
		AX_CORE_ASSERT(vkAllocateMemory(device.getHandle(), &allocateInfo, nullptr, &memory) == VK_SUCCESS, "Failed to allocate image memory!");
	
		vkBindImageMemory(device.getHandle(), image, memory, 0);
		shouldClean = true;
	}

	void VulkanImage::transitionLayout(VulkanCommandBuffer& commandBuffer, VkImageLayout newLayout) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getFamilyIndex();
		barrier.dstQueueFamilyIndex = device.getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)->getFamilyIndex();
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VulkanImageView::getVkImageAspectFlags(createInfo.aspectMask);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} 
		else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported layout transition!");
			return;
		}
		vkCmdPipelineBarrier(commandBuffer.getHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		currentLayout = newLayout;
	}

	void VulkanImage::copyFromBuffer(VulkanCommandBuffer& commandBuffer, VkBuffer buffer) const {
		VkBufferImageCopy copyRegion{};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VulkanImageView::getVkImageAspectFlags(createInfo.aspectMask);
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;

		copyRegion.imageOffset = { 0, 0, 0 };
		copyRegion.imageExtent = { createInfo.width, createInfo.height, 1 };
		vkCmdCopyBufferToImage(commandBuffer.getHandle(), buffer, image, currentLayout, 1, &copyRegion);
	}

	VkImageUsageFlags VulkanImage::getVkImageUsageFlags(uint32_t usage) {
		VkImageUsageFlags flags{};
		if (usage & RenderTarget) {
			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (usage & ShaderResource) {
			flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (usage & TransferDst) {
			flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (usage & TransferSrc) {
			flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (usage & DepthStencil) {
			flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		return flags;
	}
}
