#include "MetalPipeline.h"
#include "Core/Assert.h"
#include "MetalResourceLayout.h"
#include "MetalResourceSet.h"
#include "Utils/FileSystem.h"

namespace Axiom {
    MetalPipeline::MetalPipeline(const CreateInfo &createInfo, MTL::Device *device) {
        std::string shaderCode = FileSystem::readFileStr(createInfo.uniqueShaderPath);
        NS::String *shaderSource = NS::String::string(shaderCode.c_str(), NS::UTF8StringEncoding);

        MTL::CompileOptions *compileOptions = MTL::CompileOptions::alloc()->init();
        NS::Error *error = nullptr;

        MTL::Library *library = device->newLibrary(shaderSource, compileOptions, &error);
        AX_CORE_ASSERT(error == nullptr, "Failed to compile shader library: {}", error->localizedDescription()->utf8String());
        AX_CORE_ASSERT(library, "Failed to create shader library");

        compileOptions->release();
        shaderSource->release();

        MTL::Function *vertexFunction = library->newFunction(NS::String::string("vertex", NS::UTF8StringEncoding), nullptr, &error);
        AX_CORE_ASSERT(error == nullptr, "Failed to create vertex function: {}", error->localizedDescription()->utf8String());
        AX_CORE_ASSERT(vertexFunction, "Failed to create vertex function");

        MTL::Function *fragmentFunction = library->newFunction(NS::String::string("fragment", NS::UTF8StringEncoding), nullptr, &error);
        AX_CORE_ASSERT(error == nullptr, "Failed to create fragment function: {}", error->localizedDescription()->utf8String());
        AX_CORE_ASSERT(fragmentFunction, "Failed to create fragment function");

        MTL::RenderPipelineDescriptor *pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        pipelineDescriptor->setVertexFunction(vertexFunction);
        pipelineDescriptor->setFragmentFunction(fragmentFunction);

        uint32_t resourceIndex = 1; // Index 0 is reserved for push constants
        for (size_t i = 0; i < createInfo.resourceLayouts.size(); i++) {
            ResourceLayout *layout = createInfo.resourceLayouts[i];

            MTL::ArgumentEncoder *encoder = vertexFunction->newArgumentEncoder(resourceIndex);
            if (!encoder) {
                encoder = fragmentFunction->newArgumentEncoder(resourceIndex);
            }

            if (encoder) {
                argumentEncoders[layout] = encoder;
            } else {
                AX_CORE_LOG_WARN("ResourceLayout at index {} was optimized out or not found.", resourceIndex);
            }

            resourceIndex++;
        }

        vertexFunction->release();
        fragmentFunction->release();
        library->release();

        MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

        for (size_t i = 0; i < createInfo.vertexBindings.size(); i++) {
            MTL::VertexBufferLayoutDescriptor *layout = MTL::VertexBufferLayoutDescriptor::alloc()->init();
            layout->setStride(createInfo.vertexBindings[i].stride);
            layout->setStepFunction(createInfo.vertexBindings[i].inputRate == VertexInputRate::Vertex ? MTL::VertexStepFunctionPerVertex
                                                                                                      : MTL::VertexStepFunctionPerInstance);
            vertexDescriptor->layouts()->setObject(layout, createInfo.vertexBindings[i].binding);
            layout->release();
        }

        for (size_t i = 0; i < createInfo.vertexAttributes.size(); i++) {
            MTL::VertexAttributeDescriptor *attribute = MTL::VertexAttributeDescriptor::alloc()->init();
            attribute->setFormat(axToMetalVertexFormat(createInfo.vertexAttributes[i].format));
            attribute->setOffset(createInfo.vertexAttributes[i].offset);
            attribute->setBufferIndex(createInfo.vertexAttributes[i].binding);
            vertexDescriptor->attributes()->setObject(attribute, createInfo.vertexAttributes[i].location);
            attribute->release();
        }

        pipelineDescriptor->setVertexDescriptor(vertexDescriptor);
        vertexDescriptor->release();

        pipelineDescriptor->setInputPrimitiveTopology(
            createInfo.topology == PrimitiveTopology::TriangleList
                ? MTL::PrimitiveTopologyClassTriangle
                : (createInfo.topology == PrimitiveTopology::LineList ? MTL::PrimitiveTopologyClassLine : MTL::PrimitiveTopologyClassPoint));
        pipelineDescriptor->setRasterizationEnabled(createInfo.polygonMode != PolygonMode::Fill);

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

        pipelineState = device->newRenderPipelineState(pipelineDescriptor, &error);
        AX_CORE_ASSERT(error == nullptr, "Failed to create render pipeline state: {}", error->localizedDescription()->utf8String());
        AX_CORE_ASSERT(pipelineState, "Failed to create render pipeline state");
        pipelineDescriptor->release();
    }

    MetalPipeline::~MetalPipeline() {
        for (auto &pair : argumentEncoders) {
            pair.second->release();
        }
        argumentEncoders.clear();
        if (pipelineState) {
            pipelineState->release();
            pipelineState = nullptr;
        }
    }

    std::unique_ptr<ResourceSet> MetalPipeline::createResourceSet(ResourceLayout *resourceLayout) {
        auto it = argumentEncoders.find(resourceLayout);
        AX_CORE_ASSERT(it != argumentEncoders.end(), "Resource layout not found in pipeline's argument encoders");

        return std::make_unique<MetalResourceSet>(resourceLayout, it->second);
    }
} // namespace Axiom