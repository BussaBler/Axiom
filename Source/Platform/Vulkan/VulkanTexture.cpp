#include "VulkanTexture.h"

namespace Axiom {
	VulkanTexture::VulkanTexture(VulkanDevice& vkDevice, uint32_t width, uint32_t height, uint8_t channels, std::vector<uint8_t> data)
		: Texture(width, height, channels), device(vkDevice) {
		VkDeviceSize imageSize = static_cast<VkDeviceSize>(width * height * channels);
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VulkanBuffer stagingBuffer(device, usage, imageSize, properties, true);
		stagingBuffer.copyFrom(data.data(), imageSize, 0);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		image = std::make_unique<VulkanImage>(device, imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocateAndBeginSingleUse(device.getGraphicsCommandPool());
		image->transitionImageLayout(commandBuffer, device.getGraphicsQueue(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image->copyFromBuffer(commandBuffer, stagingBuffer.getHandle());
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

		AX_CORE_ASSERT(vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &sampler) == VK_SUCCESS, "Failed to create texture sampler!");
		generation++;
	}

	VulkanTexture::~VulkanTexture() {
		vkDeviceWaitIdle(device.getHandle());
		vkDestroySampler(device.getHandle(), sampler, nullptr);
	}
}
