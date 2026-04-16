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

        MTL::Texture* getHandle() const { return metalTexture; }

      private:
        MTL::Texture* metalTexture = nullptr;
        Format format = Format::Undefined;
    };
} // namespace Axiom