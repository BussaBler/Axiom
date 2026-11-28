#pragma once
#include "MetalUtils.h"
#include "Renderer/Backend/Metal/MetalResourceLayout.h"
#include "Renderer/ResourceSet.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Axiom {
    class MetalResourceSet : public ResourceSet {
      public:
        MetalResourceSet(const ResourceLayout* layout, MTL::Device* device);
        ~MetalResourceSet() override;

        void update(const std::vector<Binding>& bindings) override;

        MTL::Buffer* getArgumentBuffer() const { return argumentBuffer; }
        NS::UInteger getFragmentArgumentBufferOffset() const { return fragmentArgumentBufferOffset; }
        const std::vector<MTL::Resource*>& getResidentResources() {
            if (isResidencyDirty) {
                residentResources.clear();
                for (const auto& [index, resource] : resourceAtIndex) {
                    residentResources.push_back(resource);
                }
                isResidencyDirty = false;
            }
            return residentResources;
        }

      private:
        const MetalResourceLayout* resourceLayout;
        MTL::ArgumentEncoder* vertexArgumentEncoder = nullptr;
        MTL::ArgumentEncoder* fragmentArgumentEncoder = nullptr;
        MTL::Buffer* argumentBuffer = nullptr;
        NS::UInteger fragmentArgumentBufferOffset = 0;

        std::unordered_map<uint32_t, MTL::Resource*> resourceAtIndex;
        std::vector<MTL::Resource*> residentResources;
        bool isResidencyDirty = false;
    };
} // namespace Axiom
