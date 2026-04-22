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
        uint32_t bindingOffset = 0;
        residentBuffers.clear();
        residentTextures.clear();
        for (const auto& binding : bindings) {
            switch (binding.type) {
            case ResourceType::UniformBuffer:
            case ResourceType::StorageBuffer: {
                std::vector<MTL::Buffer*> buffersToSet;
                std::vector<NS::UInteger> bufferOffsets;
                for (Buffer* buffer : binding.buffers) {
                    MetalBuffer* metalBuffer = static_cast<MetalBuffer*>(buffer);
                    buffersToSet.push_back(metalBuffer->getHandle());
                    residentBuffers.push_back(metalBuffer->getHandle());
                    bufferOffsets.push_back(0); // TODO: support buffer offsets
                }
                argumentEncoder->setBuffers(buffersToSet.data(), bufferOffsets.data(), NS::Range::Make(binding.binding + bindingOffset, buffersToSet.size()));
                bindingOffset += binding.maxNumberOfResources - 1;
                break;
            }
            case ResourceType::Texture: {
                std::vector<MTL::Texture*> texturesToSet;
                for (Texture* texture : binding.textures) {
                    MetalTexture* metalTexture = static_cast<MetalTexture*>(texture);
                    texturesToSet.push_back(metalTexture->getHandle());
                    residentTextures.push_back(metalTexture->getHandle());
                }
                argumentEncoder->setTextures(texturesToSet.data(), NS::Range::Make(binding.binding + bindingOffset, texturesToSet.size()));
                bindingOffset += binding.maxNumberOfResources - 1;
                break;
            }
            case ResourceType::Sampler: {
                std::vector<MTL::SamplerState*> samplersToSet;
                for (Sampler* sampler : binding.samplers) {
                    MetalSampler* metalSampler = static_cast<MetalSampler*>(sampler);
                    samplersToSet.push_back(metalSampler->getHandle());
                }
                argumentEncoder->setSamplerStates(samplersToSet.data(), NS::Range::Make(binding.binding + bindingOffset, samplersToSet.size()));
                bindingOffset += binding.maxNumberOfResources - 1;
                break;
            }
            default:
                AX_CORE_LOG_ERROR("Unsupported resource type in MetalResourceSet::update");
                break;
            }
        }
    }
} // namespace Axiom