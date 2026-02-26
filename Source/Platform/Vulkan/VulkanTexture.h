#pragma once
#include "Renderer/Texture.h"
#include "Core/Assert.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;
	class VulkanImage;

	class VulkanTexture : public Texture {
	public:
		VulkanTexture(VulkanDevice& vkDevice);
		~VulkanTexture() override;

		void init(TextureCreateInfo& createInfo);

		Vk::Sampler getHandle() const { return sampler; }
		Vk::ImageView getImageView() const;

	private:
		VulkanDevice& device;
		std::unique_ptr<VulkanImage> image;
		ResourceViewCreateInfo imageViewCreateInfo;
		Vk::Sampler sampler = nullptr;
	};
}