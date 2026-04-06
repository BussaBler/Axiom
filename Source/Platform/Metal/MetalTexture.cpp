#include "MetalTexture.h"

namespace Axiom {
    MetalTexture::MetalTexture(const CreateInfo &createInfo, MTL::Device *device) {
        MTL::TextureDescriptor *descriptor =
            MTL::TextureDescriptor::texture2DDescriptor(axToMetalPixelFormat(createInfo.format), createInfo.width, createInfo.height, createInfo.mipLevels > 1);
        descriptor->setUsage(axToMetalTextureUsage(createInfo.usage));
        descriptor->setStorageMode(axToMetalStorageMode(createInfo.memoryUsage));

        metalTexture = device->newTexture(descriptor);
        descriptor->release();
    }

    MetalTexture::~MetalTexture() {
        if (metalTexture) {
            metalTexture->release();
            metalTexture = nullptr;
        }
    }
} // namespace Axiom