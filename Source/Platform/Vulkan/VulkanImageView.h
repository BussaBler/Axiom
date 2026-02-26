#pragma once
#include "VulkanView.h"
#include "VulkanImage.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanImageView : public VulkanView {
	public:
		VulkanImageView(VulkanDevice& vkDevice, VulkanImage& vkImage) : VulkanView(vkDevice), image(vkImage), imageView(VK_NULL_HANDLE) {}
		~VulkanImageView() override;

		void init(const ResourceViewCreateInfo& resourceViewCreateInfo);

		Vk::ImageView getHandle() const { return imageView; }

		static Vk::ImageAspectFlags getVkImageAspectFlags(uint32_t format);

	private:
		VulkanImage& image;
		Vk::ImageView imageView;
	};
}

