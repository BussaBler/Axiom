#pragma once
#include "MetalTexture.h"
#include "MetalUtils.h"
#include "Renderer/SwapChain.h"

namespace Axiom {
    class MetalSwapChain : public SwapChain {
      public:
        MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height);
        ~MetalSwapChain() override;

        bool acquireNextImage() override;
        Texture* getCurrentTexture() override;
        Format getTextureFormat() const override;
        bool present() override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

      private:
        MTL::Device* device = nullptr;
        CA::MetalLayer* metalLayer = nullptr;
        CA::MetalDrawable* currentDrawable = nullptr;
        std::unique_ptr<MetalTexture> currentDrawableTexture = nullptr;
        Format textureFormat = Format::Undefined;
    };
} // namespace Axiom