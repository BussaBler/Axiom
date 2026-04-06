#include "MetalSampler.h"

namespace Axiom {
    MetalSampler::MetalSampler(const CreateInfo& createInfo, MTL::Device* device) {
        MTL::SamplerDescriptor* samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
        samplerDescriptor->setSAddressMode(axToMetalAdressMode(createInfo.adressMode));
        samplerDescriptor->setTAddressMode(axToMetalAdressMode(createInfo.adressMode));
        samplerDescriptor->setRAddressMode(axToMetalAdressMode(createInfo.adressMode));
        samplerDescriptor->setMinFilter(axToMetalFilterMode(createInfo.filterMode));
        samplerDescriptor->setMagFilter(axToMetalFilterMode(createInfo.filterMode));
        samplerDescriptor->setMipFilter(axToMetalMipmapFilterMode(createInfo.mipmapFilterMode));
        samplerState = device->newSamplerState(samplerDescriptor);
        samplerDescriptor->release();
    }

    MetalSampler::~MetalSampler() {
        if (samplerState) {
            samplerState->release();
            samplerState = nullptr;
        }
    }
} // namespace Axiom