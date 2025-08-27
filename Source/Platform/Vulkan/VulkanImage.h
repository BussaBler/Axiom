#pragma once
#include "Core/Assert.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	class VulkanImage {
	public:
		VulkanImage(VulkanDevice& vkDevice, const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags memoryFlags, VkImageAspectFlagBits aspectFlags);
		~VulkanImage();

		void transitionImageLayout(VulkanCommandBuffer commandBuffer, VkQueue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyFromBuffer(VulkanCommandBuffer commandBuffer, VkBuffer buffer) const;

		VkImage getImage() const { return image; }
		VkDeviceMemory getImageMemory() const { return imageMemory; }
		VkImageView getImageView() const { return imageView; }

	private:
		void createImageView(VulkanDevice& device, VkFormat format, VkImageAspectFlagBits aspectFlags);

	private:
		VulkanDevice& device;
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		uint32_t width, height;
	};
}

