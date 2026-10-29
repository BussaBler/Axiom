#pragma once
#include "MetalTexture.h"
#include "MetalUtils.h"
#include "Renderer/SwapChain.h"

namespace Axiom {
    class MetalSwapChain : public SwapChain {
      public:
        struct CreateInfo {
            MTL::Device* metalDevice;
            CA::MetalLayer* metalLayer;
            uint32_t width;
            uint32_t height;
            uint32_t maxFramesInFlight;
            Format imageFormat;
        };

      public:
        MetalSwapChain(const CreateInfo& createInfo);
        ~MetalSwapChain() override;

        bool acquireNextImage() override;
        Texture* getCurrentTexture() override;
        Texture* getCurrentDepthTexture() override;
        Format getDepthTextureFormat() const override;
        bool present() override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;
        uint32_t getFrameCount() const override;
        uint32_t getCurrentFrameIndex() const override;
        CA::MetalDrawable* getCurrentDrawable() const { return currentDrawable; }
        void setCurrentFrameIndex(uint32_t frame) { currentFrameIndex = frame; }

      private:
        MTL::Device* device = nullptr;
        CA::MetalLayer* metalLayer = nullptr;
        CA::MetalDrawable* currentDrawable = nullptr;
        std::unique_ptr<MetalTexture> currentDrawableTexture = nullptr;
        std::vector<std::unique_ptr<MetalTexture>> depthTextures;
        Format depthTextureFormat = Format::Undefined;

        uint32_t maxFramesInFlight = 0;
        uint32_t currentFrameIndex = 0;
    };
} // namespace Axiom
