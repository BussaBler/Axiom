#pragma once
#include "Core/Assert.h"
#include "VulkanDevice.h"

namespace Axiom {
	class VulkanImage {
	public:
		VulkanImage() = default;
		~VulkanImage() = default;

		void create(VulkanDevice& vkDevice, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspectFlags);
		void destroy(VulkanDevice& device);

		VkImage getImage() const { return image; }
		VkDeviceMemory getImageMemory() const { return imageMemory; }
		VkImageView getImageView() const { return imageView; }

	private:
		void createImageView(VulkanDevice& device, VkFormat format, VkImageAspectFlagBits aspectFlags);

	private:
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};
}

