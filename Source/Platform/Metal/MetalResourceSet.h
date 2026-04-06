#pragma once
#include "MetalUtils.h"
#include "Renderer/ResourceSet.h"

namespace Axiom {
    class MetalResourceSet : public ResourceSet {
      public:
        MetalResourceSet(ResourceLayout *layout, MTL::Device *device, MTL::ArgumentEncoder *argumentEncoder);
        ~MetalResourceSet() override;

        void update(const std::vector<Binding> &bindings) override;

      private:
        MTL::Buffer *argumentBuffer = nullptr;
        MTL::ArgumentEncoder *argumentEncoder = nullptr;
    };
} // namespace Axiom