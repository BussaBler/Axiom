#pragma once
#include "Renderer/Texture.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h" 

namespace Axiom {
    class VulkanTexture : public Texture {
	public:
		VulkanTexture(VulkanDevice& vkDevice, uint32_t width, uint32_t height, uint8_t channels, std::vector<uint8_t> data);
		~VulkanTexture();

		VkImageView getImageView() const { return image->getImageView(); }
		VkSampler getSampler() const { return sampler; }

	private:
		VulkanDevice& device;
		std::unique_ptr<VulkanImage> image;
		VkSampler sampler;
    };
}

