#include "axpch.h"

#include "MetalSwapChain.h"

namespace Axiom {
    MetalSwapChain::MetalSwapChain(MTL::Device* metalDevice, CA::MetalLayer* metalLayer, uint32_t width, uint32_t height, uint32_t maxFramesInFlight)
        : SwapChain(), device(metalDevice), metalLayer(metalLayer), maxFramesInFlight(maxFramesInFlight) {
        metalLayer->setDrawableSize({static_cast<CGFloat>(width), static_cast<CGFloat>(height)});
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
        metalLayer->setFramebufferOnly(true);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        metalLayer->setColorspace(colorSpace);
        CGColorSpaceRelease(colorSpace);
        textureFormat = Format::B8G8R8A8Srgb;
        depthTextureFormat = Format::D32sFloat;

        depthTextures.resize(maxFramesInFlight);
        Texture::CreateInfo depthTextureCreateInfo = {
            .width = width,
            .height = height,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = depthTextureFormat,
            .usage = TextureUsage::DepthStencilAttachment,
            .aspect = TextureAspect::Depth,
            .memoryUsage = MemoryUsage::GPUOnly,
        };

        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
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
