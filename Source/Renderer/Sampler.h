#pragma once
#include "Core/Base.h"

namespace Axiom {
    AX_ENUM() enum class SamplerAddressMode { Repeat, MirroredRepeat, ClampToEdge, ClampToBorder };

    AX_ENUM() enum class SamplerFilterMode { Linear, Nearest };

    class Sampler {
      public:
        Sampler() = default;
        virtual ~Sampler() = default;

        struct CreateInfo {
            SamplerAddressMode adressMode = SamplerAddressMode::Repeat;
            SamplerFilterMode filterMode = SamplerFilterMode::Linear;
            SamplerFilterMode mipmapFilterMode = SamplerFilterMode::Linear;
        };
    };
} // namespace Axiom
