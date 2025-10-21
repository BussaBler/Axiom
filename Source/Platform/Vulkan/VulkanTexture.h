#pragma once
#include "Renderer/Texture.h"
#include "Core/Assert.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanImage;

	class VulkanTexture : public Texture {
	public:
		VulkanTexture(VulkanDevice& vkDevice);
		~VulkanTexture() override;

		void init(TextureCreateInfo& createInfo);

		VkSampler getHandle() const { return sampler; }
		VkImageView getImageView() const;

	private:
		VulkanDevice& device;
		std::unique_ptr<VulkanImage> image;
		ResourceViewCreateInfo imageViewCreateInfo;
		VkSampler sampler = VK_NULL_HANDLE;
	};
}

