#include "MetalSwapChain.h"

namespace Axiom {
    MetalSwapChain::MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height)
        : SwapChain(), device(metalDevice), metalLayer(metalLayer) {
        metalLayer->setDrawableSize({static_cast<CGFloat>(width), static_cast<CGFloat>(height)});
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        metalLayer->setFramebufferOnly(true);
        textureFormat = Format::B8G8R8A8Unorm;
        depthTextureFormat = Format::D32sFloat;

        depthTextures.resize(metalLayer->maximumDrawableCount());
        Texture::CreateInfo depthTextureCreateInfo = {
            .width = width,
            .height = height,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = depthTextureFormat,
            .usage = TextureUsage::DepthStencilAttachment,
            .aspect = TextureAspect::Depth,
            .initialState = TextureState::Undefined,
            .memoryUsage = MemoryUsage::GPUOnly,
        };

        for (size_t i = 0; i < depthTextures.size(); i++) {
            depthTextures[i] = std::make_unique<MetalTexture>(depthTextureCreateInfo, device);
        }
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

        currentFrameIndex = (currentFrameIndex + 1) % depthTextures.size();

        return true;
    }

    Texture* MetalSwapChain::getCurrentTexture() {
        return currentDrawableTexture.get();
    }

    Texture* MetalSwapChain::getCurrentDepthTexture() {
        return depthTextures[currentFrameIndex].get();
    }

    Format MetalSwapChain::getDepthTextureFormat() const {
        return depthTextureFormat;
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

    uint32_t MetalSwapChain::getFrameCount() const {
        return depthTextures.size();
    }

    uint32_t MetalSwapChain::getCurrentFrameIndex() const {
        return currentFrameIndex;
    }
} // namespace Axiom