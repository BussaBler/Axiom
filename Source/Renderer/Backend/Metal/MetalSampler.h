#pragma once
#include "MetalUtils.h"
#include "Renderer/Sampler.h"

namespace Axiom {
    class MetalSampler : public Sampler {
      public:
        MetalSampler(const CreateInfo& createInfo, MTL::Device* device);
        ~MetalSampler() override;

        MTL::SamplerState* getHandle() const {
            return samplerState;
        }

      private:
        MTL::SamplerState* samplerState = nullptr;
    };
} // namespace Axiom