#pragma once
#include "Buffer.h"
#include "ResourceLayout.h"
#include "Sampler.h"
#include "Texture.h"

namespace Axiom {
    class ResourceSet {
      public:
        struct Binding {
            uint32_t binding = 0;
            ResourceType type = ResourceType::UniformBuffer;
            Buffer* buffer = nullptr;
            Texture* texture = nullptr;
            Sampler* sampler = nullptr;
        };

      public:
        ResourceSet() = default;
        virtual ~ResourceSet() = default;

        virtual void update(const std::vector<Binding>& bindings) = 0;
    };
} // namespace Axiom
