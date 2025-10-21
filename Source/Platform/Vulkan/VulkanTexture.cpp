#include "axpch.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanTexture.h"

namespace Axiom {
	VulkanTexture::VulkanTexture(VulkanDevice& vkDevice) : device(vkDevice) {
	
	}

	VulkanTexture::~VulkanTexture() {
		if (sampler != VK_NULL_HANDLE) {
			vkDestroySampler(device.getHandle(), sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}
	}

	void VulkanTexture::init(TextureCreateInfo& createInfo) {
		image = std::make_unique<VulkanImage>(device);
		ResourceCreateInfo imageCreateInfo;
		imageCreateInfo.width = createInfo.width;
		imageCreateInfo.height = createInfo.height;
		imageCreateInfo.format = ResourceFormat::RGBA8_U;
		imageCreateInfo.usage = ResourceUsage::TransferDst | ResourceUsage::TransferSrc | ResourceUsage::ShaderResource;
		imageCreateInfo.memoryUsage = ResourceMemoryUsage::GPU_Only;
		imageCreateInfo.type = ResourceType::Image;
		image->init(imageCreateInfo);
		ResourceViewCreateInfo viewCreateInfo{};
		viewCreateInfo.format = ResourceFormat::RGBA8_U;
		viewCreateInfo.aspectMask = ResourceAspectMask::Color;
		image->getView(viewCreateInfo);
		image->loadData(createInfo.data, static_cast<uint64_t>(createInfo.width) * createInfo.height * createInfo.channels);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		AX_CORE_ASSERT(vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &sampler) == VK_SUCCESS, "Failed to create texture sampler!");
	}

	VkImageView VulkanTexture::getImageView() const {
		return static_cast<VulkanImageView&>(image->getView(imageViewCreateInfo)).getHandle();
	}
}