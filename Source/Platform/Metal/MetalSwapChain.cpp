#include "MetalSwapChain.h"

namespace Axiom {
    MetalSwapChain::MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height)
        : SwapChain(), device(metalDevice), metalLayer(metalLayer) {
        metalLayer->setDrawableSize({static_cast<CGFloat>(width), static_cast<CGFloat>(height)});
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        metalLayer->setFramebufferOnly(true);
        textureFormat = Format::B8G8R8A8Unorm;
    }

    MetalSwapChain::~MetalSwapChain() {
    }

    bool MetalSwapChain::acquireNextImage() {
        CA::MetalDrawable* drawable = metalLayer->nextDrawable();

        if (!drawable) {
            return false;
        }

        currentDrawable = drawable->retain();
        currentDrawableTexture = std::make_unique<MetalTexture>(currentDrawable->texture());
        textureFormat = currentDrawableTexture->getFormat();
        return true;
    }

    Texture* MetalSwapChain::getCurrentTexture() {
        return currentDrawableTexture.get();
    }

    Format MetalSwapChain::getTextureFormat() const {
        return textureFormat;
    }

    bool MetalSwapChain::present() {
        if (currentDrawable) {
            currentDrawable->present();
            currentDrawable->release();
            currentDrawable = nullptr;
            currentDrawableTexture.reset();
        }

        return true;
    }

    uint32_t MetalSwapChain::getWidth() const {
        return metalLayer->drawableSize().width;
    }

    uint32_t MetalSwapChain::getHeight() const {
        return metalLayer->drawableSize().height;
    }
} // namespace Axiom