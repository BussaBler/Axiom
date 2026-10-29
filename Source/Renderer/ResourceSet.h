#pragma once
#include "Buffer.h"
#include "ResourceLayout.h"
#include "Sampler.h"
#include "Texture.h"

#include <cstdint>
#include <span>
#include <variant>

namespace Axiom {
    namespace Bindings {
        constexpr uint32_t GlobalDataSet = 0;
        constexpr uint32_t BindlessTextureSet = 2;

        constexpr uint32_t GlobalDataBinding = 0;
        constexpr uint32_t BindlessTextureBinding = 0;
        constexpr uint32_t BindlessTextureSamplersBinding = 1;
    } // namespace Bindings

    class ResourceSet {
      public:
        struct Binding {
            uint32_t binding = 0;
            ResourceType type = ResourceType::UniformBuffer;
            ShaderStage stages = ShaderStage::None;
            std::variant<std::span<Buffer*>, std::span<Texture*>, std::span<Sampler*>> data;
            uint32_t dataOffset = 0;
        };

      public:
        ResourceSet() = default;
        virtual ~ResourceSet() = default;

        virtual void update(const std::vector<Binding>& bindings) = 0;
    };
} // namespace Axiom
