#include "VulkanTexture.h"

namespace Axiom {
	VulkanTexture::VulkanTexture(VulkanDevice& vkDevice) : Texture(), device(vkDevice) {

	}

	VulkanTexture::~VulkanTexture() {
		destroyData();
	}

	void VulkanTexture::createData(uint32_t width, uint32_t height, uint8_t channels, const std::vector<uint8_t>& data) {
		this->width = width;
		this->height = height;
		this->channels = channels;

		VkDeviceSize imageSize = static_cast<VkDeviceSize>(width * height * channels);
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VulkanBuffer stagingBuffer(device, usage, imageSize, properties, true);
		stagingBuffer.copyFrom(data.data(), imageSize, 0);

		ImageCreateInfo imageInfo{};
		imageInfo.vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.vkImageCreateInfo.extent.width = width;
		imageInfo.vkImageCreateInfo.extent.height = height;
		imageInfo.vkImageCreateInfo.extent.depth = 1;
		imageInfo.vkImageCreateInfo.mipLevels = 1;
		imageInfo.vkImageCreateInfo.arrayLayers = 1;
		imageInfo.vkImageCreateInfo.format = format;
		imageInfo.vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		imageInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		image = std::make_unique<VulkanImage>(device, imageInfo);
		
		ImageViewCreateInfo viewInfo{};
		viewInfo.format = format;
		viewInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		imageView = std::make_unique<VulkanImageView>(device, *image, viewInfo);

		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocateAndBeginSingleUse(device.getGraphicsCommandPool());
		image->transitionImageLayout(commandBuffer, device.getGraphicsQueue(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image->copyFromBuffer(commandBuffer, stagingBuffer);
		image->transitionImageLayout(commandBuffer, device.getGraphicsQueue(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		commandBuffer.endAndFreeSingleUse(device.getGraphicsCommandPool(), device.getGraphicsQueue());

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		AX_CORE_ASSERT(vkCreateSampler(device.getHandle<VkDevice>(), &samplerInfo, nullptr, &sampler) == VK_SUCCESS, "Failed to create texture sampler!");
		generation++;
	}

	void VulkanTexture::destroyData() {
		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		vkDestroySampler(device.getHandle<VkDevice>(), sampler, nullptr);
	}
}
