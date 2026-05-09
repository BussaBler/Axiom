#pragma once
#include "Asset/UUID.h"
#include "Core/Base.h"
#include "Math/Color.h"
#include "Renderer/Sampler.h"

namespace Axiom {
    AX_COMPONENT() struct Sprite2DComponent {
        UUID textureId = 0;
        SamplerAddressMode textureAddressMode = SamplerAddressMode::Repeat;
        SamplerFilterMode textureFilterMode = SamplerFilterMode::Linear;
        Color color = Color::white();
    };
} // namespace Axiom