#include "axpch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanBuffer.h"

namespace Axiom {
	VulkanImage::VulkanImage(VulkanDevice& vkDevice, Vk::Image vkImage, Vk::ImageLayout vkCurrentLayout)
		: VulkanResource(vkDevice), image(vkImage), currentLayout(vkCurrentLayout), format(Vk::Format::eUndefined) {}

	VulkanImage::~VulkanImage() {
		if (image && shouldClean) {
			device.getHandle().destroyImage(image);
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
		commandBuffer.allocateAndBeginSingleUse(device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getCommandPool());
		transitionLayout(commandBuffer, Vk::ImageLayout::eTransferDstOptimal);
		copyFromBuffer(commandBuffer, stagingBuffer.getHandle());
		transitionLayout(commandBuffer, Vk::ImageLayout::eShaderReadOnlyOptimal);
		commandBuffer.endSingleUse(device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getHandle(), device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getCommandPool());
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
		Vk::ImageCreateInfo imageCreateInfo(
			{},
			Vk::ImageType::e2D, // TODO: make this configurable
			format,
			{ resourceCreateInfo.width, resourceCreateInfo.height, 1 },
			1, // TODO: make this configurable
			1, // TODO: make this configurable
			Vk::SampleCountFlagBits::e1,
			Vk::ImageTiling::eOptimal, // TODO: make this configurable
			getVkImageUsageFlags(resourceCreateInfo.usage),
			Vk::SharingMode::eExclusive,
			{}
		);
		Vk::ResultValue<Vk::Image> imageResult = device.getHandle().createImage(imageCreateInfo);

		AX_CORE_ASSERT(imageResult.result == Vk::Result::eSuccess, "Failed to create image!");
		image = imageResult.value;

		Vk::MemoryRequirements memoryRequirements = device.getHandle().getImageMemoryRequirements(image);
		memorySize = memoryRequirements.size;
		Vk::MemoryAllocateInfo allocateInfo(memoryRequirements.size, device.getAdapter().findMemoryType(memoryRequirements.memoryTypeBits, Vk::MemoryPropertyFlagBits::eDeviceLocal)); // TODO: make this configurable
		Vk::ResultValue<Vk::DeviceMemory> memoryResult = device.getHandle().allocateMemory(allocateInfo);

		AX_CORE_ASSERT(memoryResult.result == Vk::Result::eSuccess, "Failed to allocate image memory!");
		memory = memoryResult.value;

		AX_CORE_ASSERT(device.getHandle().bindImageMemory(image, memory, 0) == Vk::Result::eSuccess, "Failed to bind image memory!");
		shouldClean = true;
	}

	void VulkanImage::transitionLayout(VulkanCommandBuffer& commandBuffer, Vk::ImageLayout newLayout) {
		Vk::ImageMemoryBarrier barrier(
			{},
			{},
			currentLayout,
			newLayout,
			device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getFamilyIndex(),
			device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute)->getFamilyIndex(),
			image,
			{
				VulkanImageView::getVkImageAspectFlags(createInfo.aspectMask),
				0,
				1,
				0,
				1
			}
		);

		Vk::PipelineStageFlags sourceStage;
		Vk::PipelineStageFlags destinationStage;

		if (currentLayout == Vk::ImageLayout::eUndefined && newLayout == Vk::ImageLayout::eTransferDstOptimal) {
			barrier.setSrcAccessMask(Vk::AccessFlags{});
			barrier.setDstAccessMask(Vk::AccessFlagBits::eTransferWrite);
			sourceStage = Vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = Vk::PipelineStageFlagBits::eTransfer;
		} 
		else if (currentLayout == Vk::ImageLayout::eTransferDstOptimal && newLayout == Vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.setSrcAccessMask(Vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(Vk::AccessFlagBits::eShaderRead);
			sourceStage = Vk::PipelineStageFlagBits::eTransfer;
			destinationStage = Vk::PipelineStageFlagBits::eFragmentShader;
		}
		else {
			AX_CORE_LOG_ERROR("Unsupported layout transition!");
			return;
		}

		commandBuffer.getHandle().pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
		currentLayout = newLayout;
	}

	void VulkanImage::copyFromBuffer(VulkanCommandBuffer& commandBuffer, Vk::Buffer buffer) const {
		Vk::BufferImageCopy copyRegion{};
		copyRegion.setBufferOffset(0);
		copyRegion.setBufferRowLength(0);
		copyRegion.setBufferImageHeight(0);

		copyRegion.setImageSubresource({ VulkanImageView::getVkImageAspectFlags(createInfo.aspectMask) , 0, 0, 1 });
		copyRegion.setImageOffset({ 0, 0, 0 });
		copyRegion.setImageExtent({ createInfo.width, createInfo.height, 1 });

		commandBuffer.getHandle().copyBufferToImage(buffer, image, currentLayout, copyRegion);
	}

	Vk::ImageUsageFlags VulkanImage::getVkImageUsageFlags(uint32_t usage) {
		Vk::ImageUsageFlags flags{};
		if (usage & RenderTarget) {
			flags |= Vk::ImageUsageFlagBits::eColorAttachment;
		}
		if (usage & ShaderResource) {
			flags |= Vk::ImageUsageFlagBits::eSampled;
		}
		if (usage & TransferDst) {
			flags |= Vk::ImageUsageFlagBits::eTransferDst;
		}
		if (usage & TransferSrc) {
			flags |= Vk::ImageUsageFlagBits::eTransferSrc;
		}
		if (usage & DepthStencil) {
			flags |= Vk::ImageUsageFlagBits::eDepthStencilAttachment;
		}
		return flags;
	}
}
