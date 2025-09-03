#include "axpch.h"
#include "ImageView.h"
#include "Platform/Vulkan/VulkanImageView.h"

namespace Axiom {
	std::unique_ptr<ImageView> ImageView::create(Device& deviceRef, Image& imageRef, const ImageViewCreateInfo& createInfo) {
		return std::make_unique<VulkanImageView>(static_cast<VulkanDevice&>(deviceRef), imageRef, createInfo);
	}
}
