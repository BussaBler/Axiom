#include "MetalPipeline.h"
#include "Core/Assert.h"
#include "MetalResourceLayout.h"
#include "MetalResourceSet.h"
#include "MetalShader.h"
#include "Utils/FileSystem.h"

namespace Axiom {
    MetalPipeline::MetalPipeline(const CreateInfo& createInfo, MTL::Device* device) : device(device) {
        MetalShader* metalShader = static_cast<MetalShader*>(createInfo.shader);
        MTL::Library* library = metalShader->getLibrary();
        MTL::Function* vertexFunction = library->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));

        AX_CORE_ASSERT(vertexFunction, "Failed to create vertex function");

        MTL::Function* fragmentFunction = library->newFunction(NS::String::string("fragmentMain", NS::UTF8StringEncoding));
        AX_CORE_ASSERT(fragmentFunction, "Failed to create fragment function");

        MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        pipelineDescriptor->setVertexFunction(vertexFunction);
        pipelineDescriptor->setFragmentFunction(fragmentFunction);

        MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
        uint32_t vertexBufferOffset = 0; // index 0 to 3 are for vertex buffers

        for (size_t i = 0; i < createInfo.vertexBindings.size(); i++) {
            MTL::VertexBufferLayoutDescriptor* layout = MTL::VertexBufferLayoutDescriptor::alloc()->init();
            layout->setStride(createInfo.vertexBindings[i].stride);
            layout->setStepFunction(createInfo.vertexBindings[i].inputRate == VertexInputRate::Vertex ? MTL::VertexStepFunctionPerVertex
                                                                                                      : MTL::VertexStepFunctionPerInstance);
            vertexDescriptor->layouts()->setObject(layout, createInfo.vertexBindings[i].binding + vertexBufferOffset);
            layout->release();
        }

        for (size_t i = 0; i < createInfo.vertexAttributes.size(); i++) {
            MTL::VertexAttributeDescriptor* attribute = MTL::VertexAttributeDescriptor::alloc()->init();
            attribute->setFormat(axToMetalVertexFormat(createInfo.vertexAttributes[i].format));
            attribute->setOffset(createInfo.vertexAttributes[i].offset);
            attribute->setBufferIndex(createInfo.vertexAttributes[i].binding + vertexBufferOffset);
            vertexDescriptor->attributes()->setObject(attribute, createInfo.vertexAttributes[i].location);
            attribute->release();
        }

        pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
        vertexDescriptor->release();

        pipelineDescriptor->setInputPrimitiveTopology(
            createInfo.topology == PrimitiveTopology::TriangleList
                ? MTL::PrimitiveTopologyClassTriangle
                : (createInfo.topology == PrimitiveTopology::LineList ? MTL::PrimitiveTopologyClassLine : MTL::PrimitiveTopologyClassPoint));
        pipelineDescriptor->setRasterizationEnabled(createInfo.polygonMode == PolygonMode::Fill);

        for (size_t i = 0; i < createInfo.colorAttachmentFormats.size(); i++) {
            pipelineDescriptor->colorAttachments()->object(i)->setPixelFormat(axToMetalPixelFormat(createInfo.colorAttachmentFormats[i]));

            if (createInfo.enableBlending) {
                pipelineDescriptor->colorAttachments()->object(i)->setBlendingEnabled(true);
                pipelineDescriptor->colorAttachments()->object(i)->setRgbBlendOperation(MTL::BlendOperationAdd);
                pipelineDescriptor->colorAttachments()->object(i)->setAlphaBlendOperation(MTL::BlendOperationAdd);
                pipelineDescriptor->colorAttachments()->object(i)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                pipelineDescriptor->colorAttachments()->object(i)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                pipelineDescriptor->colorAttachments()->object(i)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                pipelineDescriptor->colorAttachments()->object(i)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            }
        }

        if (createInfo.depthAttachmentFormat != Format::Undefined) {
            pipelineDescriptor->setDepthAttachmentPixelFormat(axToMetalPixelFormat(createInfo.depthAttachmentFormat));
        }

        MTL::DepthStencilDescriptor* depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        depthStencilDescriptor->setDepthCompareFunction(createInfo.enableDepthTest ? MTL::CompareFunctionLess : MTL::CompareFunctionAlways);
        depthStencilDescriptor->setDepthWriteEnabled(createInfo.enableDepthWrite);
        depthStencilState = device->newDepthStencilState(depthStencilDescriptor);
        depthStencilDescriptor->release();

        MTL::PipelineOption options = MTL::PipelineOptionArgumentInfo | MTL::PipelineOptionBufferTypeInfo;
        MTL::RenderPipelineReflection* reflection = nullptr;

        NS::Error* error = nullptr;
        pipelineState = device->newRenderPipelineState(pipelineDescriptor, options, &reflection, &error);
        AX_CORE_ASSERT(error == nullptr, "Failed to create render pipeline state: {}", error->localizedDescription()->utf8String());
        AX_CORE_ASSERT(pipelineState, "Failed to create render pipeline state");

        NS::Array* vertexArguments = reflection->vertexArguments();
        NS::Array* fragmentArguments = reflection->fragmentArguments();

        auto hasBufferIndex = [](NS::Array* arguments, uint32_t index) {
            for (size_t i = 0; i < arguments->count(); i++) {
                MTL::Argument* arg = static_cast<MTL::Argument*>(arguments->object(i));
                if (arg->type() == MTL::ArgumentTypeBuffer && arg->index() == index) {
                    return true;
                }
            }
            return false;
        };

        uint32_t resourceIndex = 8; // index 0 to 3 is for vertex buffer, 4 is push constants, 5 to 7 are for padding, so start at 8 for global resources
        for (size_t i = 0; i < createInfo.resourceLayouts.size(); i++) {
            ResourceLayout* layout = createInfo.resourceLayouts[i];
            MTL::ArgumentEncoder* encoder = nullptr;

            if (hasBufferIndex(vertexArguments, resourceIndex)) {
                encoder = vertexFunction->newArgumentEncoder(resourceIndex);
            } else if (hasBufferIndex(fragmentArguments, resourceIndex)) {
                encoder = fragmentFunction->newArgumentEncoder(resourceIndex);
            }

            if (encoder) {
                argumentEncoders[layout] = encoder;
            } else {
                AX_CORE_LOG_WARN("ResourceLayout at global index {} was optimized out of both shader stages.", resourceIndex);
            }

            resourceIndex++;
        }

        pipelineDescriptor->release();
        vertexFunction->release();
        fragmentFunction->release();
    }

    MetalPipeline::~MetalPipeline() {
        for (auto& pair : argumentEncoders) {
            pair.second->release();
        }
        argumentEncoders.clear();
        if (pipelineState) {
            depthStencilState->release();
            pipelineState->release();
            pipelineState = nullptr;
        }
    }

    std::unique_ptr<ResourceSet> MetalPipeline::createResourceSet(ResourceLayout* resourceLayout) {
        auto it = argumentEncoders.find(resourceLayout);
        AX_CORE_ASSERT(it != argumentEncoders.end(), "Resource layout not found in pipeline's argument encoders");

        return std::make_unique<MetalResourceSet>(resourceLayout, device, it->second);
    }
} // namespace Axiom