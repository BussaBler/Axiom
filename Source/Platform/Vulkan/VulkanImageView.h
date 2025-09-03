#pragma once
#include "Renderer/Core/ImageView.h"
#include "VulkanDevice.h"

namespace Axiom {
	struct ImageViewCreateInfo {
		VkFormat format;
		VkImageAspectFlagBits aspectFlags;
	};

	class VulkanImageView : public ImageView {
	public:
		VulkanImageView(VulkanDevice& vkDevice, Image& vkImage, const ImageViewCreateInfo& createInfo);
		VulkanImageView(VulkanDevice& vkDevice, VkImage vkImage, const ImageViewCreateInfo& createInfo);
		~VulkanImageView() override;

	private:
		VulkanDevice& device;
	};
}

