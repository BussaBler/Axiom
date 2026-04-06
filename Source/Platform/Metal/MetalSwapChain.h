#pragma once
#include "MetalUtils.h"
#include "Renderer/SwapChain.h"

namespace Axiom {
    class MetalSwapChain : public SwapChain {
      public:
        MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height);
        ~MetalSwapChain() override;

        uint32_t acquireNextImage(Semaphore* imageAvailableSemaphore) override;
        Texture* getImageTexture(uint32_t index) override;
        bool present(uint32_t imageIndex, Semaphore* waitSemaphore) override;
        uint32_t getImageCount() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

      private:
        MTL::Device* device = nullptr;
        CA::MetalLayer* metalLayer = nullptr;
        CA::MetalDrawable* currentDrawable = nullptr;
    };
} // namespace Axiom