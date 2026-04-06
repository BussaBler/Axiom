#include "MetalResourceSet.h"
#include "MetalBuffer.h"
#include "MetalSampler.h"
#include "MetalTexture.h"

namespace Axiom {
    MetalResourceSet::MetalResourceSet(ResourceLayout* layout, MTL::Device* device, MTL::ArgumentEncoder* argumentEncoder) {
        this->argumentEncoder = argumentEncoder->retain();
        NS::UInteger argumentBufferLength = argumentEncoder->encodedLength();
        this->argumentBuffer = device->newBuffer(argumentBufferLength, MTL::ResourceStorageModeShared);
        this->argumentEncoder->setArgumentBuffer(this->argumentBuffer, 0);
    }

    MetalResourceSet::~MetalResourceSet() {
        if (argumentBuffer) {
            argumentBuffer->release();
            argumentBuffer = nullptr;
        }
        if (argumentEncoder) {
            argumentEncoder->release();
            argumentEncoder = nullptr;
        }
    }

    void MetalResourceSet::update(const std::vector<Binding>& bindings) {
        for (const auto& binding : bindings) {
            switch (binding.type) {
            case ResourceType::UniformBuffer:
            case ResourceType::StorageBuffer:
                MetalBuffer* buffer = static_cast<MetalBuffer*>(binding.buffer);
                argumentEncoder->setBuffer(buffer->getHandle(), 0, binding.binding);
                residentBuffers.push_back(buffer->getHandle());
                break;
            case ResourceType::Texture:
                MetalTexture* texture = static_cast<MetalTexture*>(binding.texture);
                argumentEncoder->setTexture(texture->getHandle(), binding.binding);
                residentTextures.push_back(texture->getHandle());
                break;
            case ResourceType::Sampler:
                MetalSampler* sampler = static_cast<MetalSampler*>(binding.sampler);
                argumentEncoder->setSamplerState(sampler->getHandle(), binding.binding);
                break;
            default:
                AX_CORE_LOG_ERROR("Unsupported resource type in MetalResourceSet::update");
                break;
            }
        }
    }
} // namespace Axiom