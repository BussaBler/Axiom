#include "VulkanTexture.h"

namespace Axiom {
	VulkanTexture::VulkanTexture(Vk::Device logicalDevice, const CreateInfo& createInfo) : device(logicalDevice), ownsImage(true) {
		Vk::ImageCreateInfo imageCreateInfo(
			{},
			Vk::ImageType::e2D,
			axToVkFormat(createInfo.format),
			{ createInfo.width, createInfo.height, 1 },
			createInfo.mipLevels,
			createInfo.arrayLayers,
			Vk::SampleCountFlagBits::e1,
			Vk::ImageTiling::eOptimal,
			axToVkImageUsage(createInfo.usage)
		);
		imageCreateInfo.setInitialLayout(axToVkImageLayout(createInfo.initialState));
		Vk::ResultValue<Vk::Image> imageResult = device.createImage(imageCreateInfo);
		
		AX_CORE_ASSERT(imageResult.result == Vk::Result::eSuccess, "Failed to create image for texture!");
		image = imageResult.value;

		imageAllocation = VulkanAllocator::allocate(image, axToVkMemProperty(createInfo.memoryUsage));

		Vk::Result result = device.bindImageMemory(image, imageAllocation.memory, imageAllocation.offset);
		AX_CORE_ASSERT(result == Vk::Result::eSuccess, "Failed to bind image memory");

		createImageView(imageCreateInfo.format, axToVkImageAspectFlags(createInfo.aspect));
		createSampler(Vk::Filter::eLinear, Vk::SamplerAddressMode::eRepeat, createInfo.mipLevels);
	}

	VulkanTexture::VulkanTexture(Vk::Device logicalDevice, Vk::Image existingImage) : device(logicalDevice),
			image(existingImage), ownsImage(false) {
	}

	VulkanTexture::~VulkanTexture() {
		if (ownsImage) {
			VulkanAllocator::free(imageAllocation);
			if (image) {
				device.destroyImage(image);
			}
		}
		if (imageView) {
			device.destroyImageView(imageView);
		}
		if (sampler) {
			device.destroySampler(sampler);
		}
	}

	Format VulkanTexture::getFormat() const {
		return vkToAxFormat(imageFormat);
	}

	void VulkanTexture::createImageView(Vk::Format format, Vk::ImageAspectFlags aspectFlags) {
		imageFormat = format;
		Vk::ImageViewCreateInfo createInfo(
			{},
			image,
			Vk::ImageViewType::e2D,
			format,
			{ Vk::ComponentSwizzle::eR, Vk::ComponentSwizzle::eG, Vk::ComponentSwizzle::eB, Vk::ComponentSwizzle::eA },
			{ aspectFlags, 0, 1, 0, 1 }
		);
		Vk::ResultValue<Vk::ImageView> imageViewResult = device.createImageView(createInfo);

		AX_CORE_ASSERT(imageViewResult.result == Vk::Result::eSuccess, "Failed to create image view for texture!");
		imageView = imageViewResult.value;
	}

	void VulkanTexture::createSampler(Vk::Filter filter, Vk::SamplerAddressMode addressMode, uint32_t mipLevels) {
		Vk::SamplerCreateInfo createInfo(
			{},
			filter,
			filter,
			Vk::SamplerMipmapMode::eLinear,
			addressMode,
			addressMode,
			addressMode,
			0.0f,
			false,
			1.0f,
			false,
			Vk::CompareOp::eAlways,
			0.0f,
			static_cast<float>(mipLevels)
		);
		Vk::ResultValue<Vk::Sampler> samplerResult = device.createSampler(createInfo);
		AX_CORE_ASSERT(samplerResult.result == Vk::Result::eSuccess, "Failed to create sampler for texture!");
		sampler = samplerResult.value;
	}
}
