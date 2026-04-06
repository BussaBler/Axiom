#pragma once
#include "Core/Assert.h"
#include "Renderer/Texture.h"
#include "VulkanAllocator.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanTexture : public Texture {
      public:
        VulkanTexture(Vk::Device logicalDevice, const CreateInfo &createInfo);
        VulkanTexture(Vk::Device logicalDevice, Vk::Image existingImage);
        ~VulkanTexture() override;

        Format getFormat() const override;

        void createImageView(Vk::Format format, Vk::ImageAspectFlags aspectFlags);

        inline Vk::Image getImage() const {
            return image;
        }
        inline Vk::ImageView getImageView() const {
            return imageView;
        }

      private:
        Vk::Device device = nullptr;
        Vk::Image image = nullptr;
        Vk::ImageView imageView = nullptr;
        Vk::Format imageFormat = Vk::Format::eUndefined;
        bool ownsImage = false;
        Allocation imageAllocation;
    };
} // namespace Axiom
