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
		if (sampler) {
			device.getHandle().destroySampler(sampler);
			sampler = nullptr;
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

		Vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.setMagFilter(Vk::Filter::eLinear);
		samplerInfo.setMinFilter(Vk::Filter::eLinear);
		samplerInfo.setAddressModeU(Vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeV(Vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeW(Vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAnisotropyEnable(Vk::True);
		samplerInfo.setMaxAnisotropy(16);
		samplerInfo.setBorderColor(Vk::BorderColor::eIntOpaqueBlack);
		samplerInfo.setUnnormalizedCoordinates(Vk::False);
		samplerInfo.setCompareEnable(Vk::False);
		samplerInfo.setCompareOp(Vk::CompareOp::eAlways);
		samplerInfo.setMipmapMode(Vk::SamplerMipmapMode::eLinear);
		samplerInfo.setMipLodBias(0.0f);
		samplerInfo.setMinLod(0.0f);
		samplerInfo.setMaxLod(0.0f);
		Vk::ResultValue<Vk::Sampler> samplerResult = device.getHandle().createSampler(samplerInfo);

		AX_CORE_ASSERT(samplerResult.result == Vk::Result::eSuccess, "Failed to create texture sampler!");
		sampler = samplerResult.value;
	}

	Vk::ImageView VulkanTexture::getImageView() const {
		return static_cast<VulkanImageView&>(image->getView(imageViewCreateInfo)).getHandle();
	}
}