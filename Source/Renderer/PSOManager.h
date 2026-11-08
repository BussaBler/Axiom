#pragma once

#include "Renderer/Pipeline.h"
#include "Renderer/RenderHardwareInterface.h"

#include <memory>
#include <unordered_map>

namespace Axiom {
    class PSOManager {
      public:
        PSOManager(RenderHardwareInterface& rhi) : rhi(rhi) {}
        ~PSOManager() = default;

        Pipeline* getOrCreatePipeline(const Pipeline::CreateInfo& createInfo) {
            if (pipelineCache.find(createInfo) != pipelineCache.end()) {
                return pipelineCache[createInfo].get();
            }

            pipelineCache[createInfo] = rhi.createPipeline(createInfo);
            return pipelineCache[createInfo].get();
        }

      private:
        RenderHardwareInterface& rhi;

        std::unordered_map<Pipeline::CreateInfo, std::unique_ptr<Pipeline>> pipelineCache;
    };
} // namespace Axiom
