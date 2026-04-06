#include "MetalSwapChain.h"

namespace Axiom {
    MetalSwapChain::MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height)
        : SwapChain(), device(metalDevice), metalLayer(metalLayer) {
    }

    MetalSwapChain::~MetalSwapChain() {
    }

    uint32_t MetalSwapChain::acquireNextImage(Semaphore* imageAvailableSemaphore) {
        currentDrawable = metalLayer->nextDrawable();
        return 0;
    }

    Texture* MetalSwapChain::getImageTexture(uint32_t index) {
        MTL::Texture* metalTexture = currentDrawable->texture();

        // TODO: translate MTL::Texture to Axiom::Texture

        return nullptr;
    }

    bool MetalSwapChain::present(uint32_t imageIndex, Semaphore* waitSemaphore) {
        // there is nothing to do here, I think
        return true;
    }

    uint32_t MetalSwapChain::getImageCount() const {
        return metalLayer->maximumDrawableCount();
    }

    uint32_t MetalSwapChain::getWidth() const {
        return metalLayer->drawableSize().width;
    }

    uint32_t MetalSwapChain::getHeight() const {
        return metalLayer->drawableSize().height;
    }
} // namespace Axiom