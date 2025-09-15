#include "axpch.h"
#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "VulkanImageView.h"

namespace Axiom {
	VulkanImage::VulkanImage(VulkanDevice& vkDevice, VkImage vkImage, VkImageLayout vkCurrentLayout)
		: VulkanResource(vkDevice), image(vkImage), currentLayout(vkCurrentLayout), format(VK_FORMAT_UNDEFINED) {
		id = generateId();
	}

	VulkanImage::~VulkanImage() {
		if (image && shouldClean) {
			vkDestroyImage(device.getHandle(), image, nullptr);
			image = VK_NULL_HANDLE;
		}
	}

	void VulkanImage::map(void** mappedMemory, uint32_t size, uint32_t offset) {

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
		format = getVkFormat(resourceCreateInfo.format);
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.extent = { resourceCreateInfo.width, resourceCreateInfo.height, 1 };
		imageCreateInfo.format = getVkFormat(resourceCreateInfo.format);
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

	VkImageUsageFlags VulkanImage::getVkImageUsageFlags(ResourceUsage usage) {
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
