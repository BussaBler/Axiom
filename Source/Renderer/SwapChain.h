#pragma once
#include "Texture.h"

namespace Axiom {
    class SwapChain {
      public:
        SwapChain(Format requestedFormat) : textureFormat(requestedFormat) {}
        virtual ~SwapChain() = default;

        virtual bool acquireNextImage() = 0;
        virtual Texture* getCurrentTexture() = 0;
        virtual Texture* getCurrentDepthTexture() = 0;
        virtual Format getDepthTextureFormat() const = 0;
        virtual bool present() = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual uint32_t getFrameCount() const = 0;
        virtual uint32_t getCurrentFrameIndex() const = 0;

        inline Format getTextureFormat() const { return textureFormat; }

      protected:
        Format textureFormat = Format::Undefined;
    };
} // namespace Axiom
