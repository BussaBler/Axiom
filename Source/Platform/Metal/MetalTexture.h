#pragma once
#include "MetalUtils.h"
#include "Renderer/Texture.h"

namespace Axiom {
    class MetalTexture : public Texture {
      public:
        MetalTexture(const CreateInfo &createInfo, MTL::Device *device);
        ~MetalTexture() override;

        MTL::Texture *getHandle() const {
            return metalTexture;
        }

      private:
        MTL::Texture *metalTexture = nullptr;
    };
} // namespace Axiom