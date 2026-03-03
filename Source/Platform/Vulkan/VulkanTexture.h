#pragma once
#include "Renderer/Texture.h"
#include "Core/Assert.h"
#include "VulkanInclude.h"


namespace Axiom {
	class VulkanTexture : public Texture {
	public:
		VulkanTexture(Vk::Device logicDevice, Vk::Image existingImage);
		~VulkanTexture() override;

		void createImageView(Vk::Format format, Vk::ImageAspectFlags aspectFlags);

		inline Vk::Image getImage() const { return image; }
		inline Vk::ImageView getImageView() const { return imageView; }

	private:
		Vk::Device device = nullptr;
		Vk::Image image = nullptr;
		Vk::DeviceMemory imageMemory = nullptr;
		Vk::ImageView imageView = nullptr;
		bool ownsImage = false;
	};
}

