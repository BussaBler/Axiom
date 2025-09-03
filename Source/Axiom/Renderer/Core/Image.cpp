#include "axpch.h"
#include "Image.h"
#include "Platform/Vulkan/VulkanImage.h"

namespace Axiom {
	std::unique_ptr<Image> Image::create(Device& deviceRef, const ImageCreateInfo& createInfo) {
		return std::make_unique<VulkanImage>(static_cast<VulkanDevice&>(deviceRef), createInfo);
	}
}
