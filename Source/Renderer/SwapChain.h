#pragma once
#include "Texture.h"
#include "axpch.h"

namespace Axiom {
    class SwapChain {
      public:
        SwapChain() = default;
        virtual ~SwapChain() = default;

        virtual bool acquireNextImage() = 0;
        virtual Texture* getCurrentTexture() = 0;
        virtual Format getTextureFormat() const = 0;
        virtual bool present() = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
    };
} // namespace Axiom