#include "axpch.h"

#include "MetalResourceSet.h"

#include "Core/Log.h"
#include "Metal/Metal.hpp"
#include "MetalBuffer.h"
#include "MetalSampler.h"
#include "MetalTexture.h"
#include "Renderer/Backend/Metal/MetalResourceLayout.h"
#include "Renderer/Buffer.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/Sampler.h"
#include "Renderer/Texture.h"

#include <cstdint>

namespace Axiom {
    MetalResourceSet::MetalResourceSet(const ResourceLayout* layout, MTL::Device* device) {
        const MetalResourceLayout* mtlLayout = static_cast<const MetalResourceLayout*>(layout);
        std::vector<NS::Object*> vertexDescriptors;
        std::vector<NS::Object*> fragmentDescriptors;

        for (const auto& binding : mtlLayout->getBindingsCreateInfo()) {
            MTL::ArgumentDescriptor* argDescriptor = MTL::ArgumentDescriptor::argumentDescriptor();

            argDescriptor->setIndex(binding.binding);
            argDescriptor->setArrayLength(binding.count);

            switch (binding.type) {
            case ResourceType::UniformBuffer:
                argDescriptor->setDataType(MTL::DataTypePointer);
                argDescriptor->setAccess(MTL::BindingAccessReadOnly);
                break;
            case ResourceType::StorageBuffer:
                argDescriptor->setDataType(MTL::DataTypePointer);
                argDescriptor->setAccess(MTL::BindingAccessReadWrite);
                break;
            case ResourceType::Texture2D:
                argDescriptor->setDataType(MTL::DataTypeTexture);
                argDescriptor->setAccess(MTL::BindingAccessReadOnly);
                argDescriptor->setTextureType(MTL::TextureType2D);
                break;
            case Axiom::ResourceType::Texture3D:
                argDescriptor->setDataType(MTL::DataTypeTexture);
                argDescriptor->setAccess(MTL::BindingAccessReadOnly);
                argDescriptor->setTextureType(MTL::TextureType3D);
                break;
            case ResourceType::Sampler:
                argDescriptor->setDataType(MTL::DataTypeSampler);
                argDescriptor->setAccess(MTL::BindingAccessReadOnly);
                break;
            }
            if ((binding.stages & ShaderStage::Vertex) != ShaderStage::None) {
                vertexDescriptors.push_back(argDescriptor);
            }
            if ((binding.stages & ShaderStage::Fragment) != ShaderStage::None) {
                fragmentDescriptors.push_back(argDescriptor);
            }

            maxResourcesPerBinding[binding.binding] = binding.count;
        }

        NS::UInteger totalEncodedLength = 0;

        if (!vertexDescriptors.empty()) {
            NS::Array* vertexDescriptorsArray = NS::Array::array(vertexDescriptors.data(), vertexDescriptors.size());
            vertexArgumentEncoder = device->newArgumentEncoder(vertexDescriptorsArray);
            totalEncodedLength += vertexArgumentEncoder->encodedLength();
        }

        if (!fragmentDescriptors.empty()) {
            NS::Array* fragmentDescriptorsArray = NS::Array::array(fragmentDescriptors.data(), fragmentDescriptors.size());
            fragmentArgumentEncoder = device->newArgumentEncoder(fragmentDescriptorsArray);

            fragmentArgumentBufferOffset = totalEncodedLength;
            totalEncodedLength += fragmentArgumentEncoder->encodedLength();
        }

        if (totalEncodedLength > 0) {
            argumentBuffer = device->newBuffer(totalEncodedLength, MTL::ResourceStorageModeShared);

            if (vertexArgumentEncoder) {
                vertexArgumentEncoder->setArgumentBuffer(argumentBuffer, 0);
            }
            if (fragmentArgumentEncoder) {
                fragmentArgumentEncoder->setArgumentBuffer(argumentBuffer, fragmentArgumentBufferOffset);
            }
        }
    }

    MetalResourceSet::~MetalResourceSet() {
        if (argumentBuffer) {
            argumentBuffer->release();
            argumentBuffer = nullptr;
        }
        if (fragmentArgumentEncoder) {
            fragmentArgumentEncoder->release();
            fragmentArgumentEncoder = nullptr;
        }
        if (vertexArgumentEncoder) {
            vertexArgumentEncoder->release();
            vertexArgumentEncoder = nullptr;
        }
    }

    void MetalResourceSet::update(const std::vector<Binding>& bindings) {
        residentBuffers.clear();
        residentTextures.clear();
        uint32_t vertexArgumentBindingOffset = 0;
        uint32_t fragmentArgumentBindingOffset = 0;

        for (const auto& binding : bindings) {
            switch (binding.type) {
            case ResourceType::UniformBuffer:
            case ResourceType::StorageBuffer: {
                std::vector<MTL::Buffer*> buffersToSet;
                std::vector<NS::UInteger> bufferOffsets;

                for (auto* buffer : std::get<std::span<Buffer*>>(binding.data)) {
                    MetalBuffer* metalBuffer = static_cast<MetalBuffer*>(buffer);
                    buffersToSet.push_back(metalBuffer->getHandle());
                    residentBuffers.push_back(metalBuffer->getHandle());
                    bufferOffsets.push_back(0); // TODO: support buffer offsets
                }

                if ((binding.stages & ShaderStage::Vertex) != ShaderStage::None) {
                    vertexArgumentEncoder->setBuffers(buffersToSet.data(), bufferOffsets.data(),
                                                      NS::Range::Make(binding.binding + binding.dataOffset + vertexArgumentBindingOffset, buffersToSet.size()));
                    vertexArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                if ((binding.stages & ShaderStage::Fragment) != ShaderStage::None) {
                    fragmentArgumentEncoder->setBuffers(
                        buffersToSet.data(), bufferOffsets.data(),
                        NS::Range::Make(binding.binding + binding.dataOffset + fragmentArgumentBindingOffset, buffersToSet.size()));
                    fragmentArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                break;
            }
            case ResourceType::Texture2D:
            case ResourceType::Texture3D: {
                std::vector<MTL::Texture*> texturesToSet;
                for (Texture* texture : std::get<std::span<Texture*>>(binding.data)) {
                    MetalTexture* metalTexture = static_cast<MetalTexture*>(texture);
                    texturesToSet.push_back(metalTexture->getHandle());
                    residentTextures.push_back(metalTexture->getHandle());
                }

                if ((binding.stages & ShaderStage::Vertex) != ShaderStage::None) {
                    vertexArgumentEncoder->setTextures(
                        texturesToSet.data(), NS::Range::Make(binding.binding + binding.dataOffset + vertexArgumentBindingOffset, texturesToSet.size()));
                    vertexArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                if ((binding.stages & ShaderStage::Fragment) != ShaderStage::None) {
                    fragmentArgumentEncoder->setTextures(
                        texturesToSet.data(), NS::Range::Make(binding.binding + binding.dataOffset + fragmentArgumentBindingOffset, texturesToSet.size()));
                    fragmentArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                break;
            }
            case ResourceType::Sampler: {
                std::vector<MTL::SamplerState*> samplersToSet;
                for (Sampler* sampler : std::get<std::span<Sampler*>>(binding.data)) {
                    MetalSampler* metalSampler = static_cast<MetalSampler*>(sampler);
                    samplersToSet.push_back(metalSampler->getHandle());
                }

                if ((binding.stages & ShaderStage::Vertex) != ShaderStage::None) {
                    vertexArgumentEncoder->setSamplerStates(
                        samplersToSet.data(), NS::Range::Make(binding.binding + binding.dataOffset + vertexArgumentBindingOffset, samplersToSet.size()));
                    vertexArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                if ((binding.stages & ShaderStage::Fragment) != ShaderStage::None) {
                    fragmentArgumentEncoder->setSamplerStates(
                        samplersToSet.data(), NS::Range::Make(binding.binding + binding.dataOffset + fragmentArgumentBindingOffset, samplersToSet.size()));
                    fragmentArgumentBindingOffset += maxResourcesPerBinding[binding.binding] - 1;
                }
                break;
            }
            default:
                AX_CORE_LOG_ERROR("Unsupported resource type in MetalResourceSet::update");
                break;
            }
        }
    }
} // namespace Axiom
