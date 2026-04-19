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
            std::vector<Buffer*> buffers;
            std::vector<Texture*> textures;
            std::vector<Sampler*> samplers;
            // TODO: think of a better way to do this, maybe use reflection
            size_t maxNumberOfResources = 1;
        };

      public:
        ResourceSet() = default;
        virtual ~ResourceSet() = default;

        virtual void update(const std::vector<Binding>& bindings) = 0;
    };
} // namespace Axiom
