#pragma once

#include "MetalUtils.h"
#include "Renderer/Texture.h"

namespace Axiom {
    class MetalTexture : public Texture {
      public:
        MetalTexture(const CreateInfo& createInfo, MTL::Device* device);
        MetalTexture(MTL::Texture* texture);
        ~MetalTexture() override;

        Format getFormat() const override;
        Math::iVec2 getSize() const override;
        uint32_t getMipLevels() const override;
        uint32_t getArrayLayers() const override;

        MTL::Texture* getHandle() const { return metalTexture; }

      private:
        MTL::Texture* metalTexture = nullptr;
        Format format = Format::Undefined;
        Math::iVec2 size = Math::iVec2(0, 0);
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
    };
} // namespace Axiom