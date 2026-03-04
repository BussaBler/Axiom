#include "VulkanTexture.h"

namespace Axiom {
	VulkanTexture::VulkanTexture(Vk::Device logicDevice, Vk::Image existingImage) : device(logicDevice),
			image(existingImage), ownsImage(false) {
	}

	VulkanTexture::~VulkanTexture() {
		if (ownsImage) {
			if (image) {
				device.destroyImage(image);
			}
			if (imageMemory) {
				device.freeMemory(imageMemory);
			}
		}
		if (imageView) {
			device.destroyImageView(imageView);
		}
	}

	Format VulkanTexture::getFormat() const {
		return VkFormatToAxFormat(imageFormat);
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
}
