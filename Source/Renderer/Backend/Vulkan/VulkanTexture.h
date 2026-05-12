#pragma once
#include "Core/Assert.h"
#include "Renderer/Texture.h"
#include "VulkanAllocator.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanTexture : public Texture {
      public:
        VulkanTexture(Vk::Device logicalDevice, const CreateInfo& createInfo);
        VulkanTexture(Vk::Device logicalDevice, Vk::Image existingImage, Vk::Format format, Math::iVec2 size);
        ~VulkanTexture() override;

        Format getFormat() const override;
        Math::iVec2 getSize() const override;
        uint32_t getMipLevels() const override;
        uint32_t getArrayLayers() const override;

        void createImageView(Vk::Format format, Vk::ImageAspectFlags aspectFlags, Vk::ImageViewType viewType);

        inline Vk::Image getImage() const { return image; }
        inline Vk::ImageView getImageView() const { return imageView; }

      private:
        Vk::Device device = nullptr;
        Vk::Image image = nullptr;
        Vk::ImageView imageView = nullptr;
        Vk::Format imageFormat = Vk::Format::eUndefined;
        Math::iVec2 size = Math::iVec2(0, 0);
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;

        bool ownsImage = false;
        Allocation imageAllocation;
    };
} // namespace Axiom
