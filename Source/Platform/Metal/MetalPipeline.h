#pragma once
#include "MetalUtils.h"
#include "Renderer/Pipeline.h"

namespace Axiom {
    class MetalPipeline : public Pipeline {
      public:
        MetalPipeline(const CreateInfo& createInfo, MTL::Device* device);
        ~MetalPipeline() override;

        std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout* resourceLayout) override;

        MTL::RenderPipelineState* getHandle() const { return pipelineState; }
        MTL::Winding getFaceWinding() const { return faceWinding; }
        MTL::CullMode getCullMode() const { return cullMode; }
        MTL::TriangleFillMode getFillMode() const { return fillMode; }

      private:
        MTL::Device* device;
        MTL::RenderPipelineState* pipelineState;
        MTL::Winding faceWinding;
        MTL::CullMode cullMode;
        MTL::TriangleFillMode fillMode;
        std::unordered_map<ResourceLayout*, MTL::ArgumentEncoder*> argumentEncoders;
    };
} // namespace Axiom