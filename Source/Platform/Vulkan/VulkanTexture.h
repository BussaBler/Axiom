#pragma once
#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanImageView.h"
#include "Renderer/Resources/Texture.h"

namespace Axiom {
    class VulkanTexture : public Texture {
	public:
		VulkanTexture(VulkanDevice& vkDevice);
		~VulkanTexture() override;

		void createData(uint32_t width, uint32_t height, uint8_t channels, const std::vector<uint8_t>& data) override;
		void destroyData() override;

		VkImageView getImageView() const { return imageView->getHandle<VkImageView>(); }
		VkSampler getSampler() const { return sampler; }

	private:

		VulkanDevice& device;
		std::unique_ptr<VulkanImage> image;
		std::unique_ptr<VulkanImageView> imageView;
		VkSampler sampler = VK_NULL_HANDLE;
    };
}

