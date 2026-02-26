#include "axpch.h"
#include "VulkanImageView.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanImageView::~VulkanImageView() {
		if (imageView) {
			device.getHandle().destroyImageView(imageView);
			imageView = VK_NULL_HANDLE;
		}
	}

	void VulkanImageView::init(const ResourceViewCreateInfo& resourceCreateInfo) {
		Vk::ImageViewCreateInfo viewCreateInfo(
			{},
			image.getHandle(),
			Vk::ImageViewType::e2D, // TODO: make this configurable
			VulkanResource::getVkFormat(resourceCreateInfo.format),
			{ Vk::ComponentSwizzle::eR, Vk::ComponentSwizzle::eG, Vk::ComponentSwizzle::eB, Vk::ComponentSwizzle::eA },
			{
				getVkImageAspectFlags(resourceCreateInfo.aspectMask),
				0,
				1,
				0,
				1
			}
		);
		Vk::ResultValue<Vk::ImageView> imageViewResult = device.getHandle().createImageView(viewCreateInfo);

		AX_CORE_ASSERT(imageViewResult.result == Vk::Result::eSuccess, "Failed to create image view!");
		imageView = imageViewResult.value;
	}

	Vk::ImageAspectFlags VulkanImageView::getVkImageAspectFlags(uint32_t format) {
		Vk::ImageAspectFlags flags{};
		if (format & ResourceAspectMask::Color) {
			flags |= Vk::ImageAspectFlagBits::eColor;
		}
		if (format & ResourceAspectMask::Depth) {
			flags |= Vk::ImageAspectFlagBits::eDepth;
		}
		if (format & ResourceAspectMask::Stencil) {
			flags |= Vk::ImageAspectFlagBits::eStencil;
		}
		return flags;
	}
}
