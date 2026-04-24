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

        MTL::Texture* getHandle() const { return metalTexture; }

      private:
        MTL::Texture* metalTexture = nullptr;
        Format format = Format::Undefined;
        Math::iVec2 size = Math::iVec2(0, 0);
    };
} // namespace Axiom