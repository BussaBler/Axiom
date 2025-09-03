#pragma once
#include "Axiom/Renderer/Core/Image.h"
#include "Core/Assert.h"
#include "Renderer/Core/Buffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

namespace Axiom {
	struct ImageCreateInfo {
		VkImageCreateInfo vkImageCreateInfo;
		VkMemoryPropertyFlags memoryFlags;
		VkImageAspectFlagBits aspectFlags;
	};

	class VulkanImage : public Image {
	public:
		VulkanImage(VulkanDevice& vkDevice, const ImageCreateInfo& createInfo);
		~VulkanImage();

		void transitionImageLayout(CommandBuffer& commandBuffer, Queue& queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyFromBuffer(CommandBuffer& commandBuffer, Buffer& buffer) const;

	private:
		VulkanDevice& device;
		VkDeviceMemory imageMemory;
		uint32_t width, height;
	};
}

