#include "MetalTexture.h"

namespace Axiom {
    MetalTexture::MetalTexture(const CreateInfo& createInfo, MTL::Device* device) {
        format = createInfo.format;
        size = Math::iVec2(createInfo.width, createInfo.height);
        MTL::TextureDescriptor* descriptor =
            MTL::TextureDescriptor::texture2DDescriptor(axToMetalPixelFormat(createInfo.format), createInfo.width, createInfo.height, createInfo.mipLevels > 1);
        descriptor->setUsage(axToMetalTextureUsage(createInfo.usage));
        descriptor->setStorageMode(axToMetalStorageMode(createInfo.memoryUsage));

        metalTexture = device->newTexture(descriptor);
        descriptor->release();
    }

    MetalTexture::MetalTexture(MTL::Texture* texture) : metalTexture(texture) {
        if (metalTexture) {
            metalTexture->retain();
            format = metalToAxPixelFormat(metalTexture->pixelFormat());
            size = Math::iVec2(metalTexture->width(), metalTexture->height());
        }
    }

    MetalTexture::~MetalTexture() {
        if (metalTexture) {
            metalTexture->release();
            metalTexture = nullptr;
        }
    }

    Format MetalTexture::getFormat() const {
        return format;
    }

    Math::iVec2 MetalTexture::getSize() const {
        return size;
    }
} // namespace Axiom