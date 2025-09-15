#pragma once
#include "VulkanView.h"
#include "VulkanImage.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanImageView : public VulkanView {
	public:
		VulkanImageView(VulkanDevice& vkDevice, VulkanImage& vkImage) : VulkanView(vkDevice), image(vkImage), imageView(VK_NULL_HANDLE) {}
		~VulkanImageView() override;

		void init(const ResourceViewCreateInfo& resourceViewCreateInfo);

		VkImageView getHandle() const { return imageView; }

		static VkImageAspectFlags getVkImageAspectFlags(uint32_t format);

	private:
		VulkanImage& image;
		VkImageView imageView;
	};
}

