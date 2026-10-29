#include "axpch.h"

#include "MetalSwapChain.h"

namespace Axiom {
    MetalSwapChain::MetalSwapChain(const CreateInfo& createInfo)
        : SwapChain(createInfo.imageFormat), device(createInfo.metalDevice), metalLayer(createInfo.metalLayer),
          maxFramesInFlight(createInfo.maxFramesInFlight) {
        metalLayer->setDrawableSize({static_cast<CGFloat>(createInfo.width), static_cast<CGFloat>(createInfo.height)});
        depthTextureFormat = Format::D32sFloat;
        metalLayer->setPixelFormat(axToMetalPixelFormat(textureFormat));
        metalLayer->setFramebufferOnly(true);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        metalLayer->setColorspace(colorSpace);
        CGColorSpaceRelease(colorSpace);

        depthTextures.resize(maxFramesInFlight);
        Texture::CreateInfo depthTextureCreateInfo = {
            .width = createInfo.width,
            .height = createInfo.height,
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
