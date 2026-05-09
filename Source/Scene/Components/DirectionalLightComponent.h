#pragma once
#include "Core/Base.h"
#include "Math/Color.h"

namespace Axiom {
    AX_COMPONENT() struct DirectionalLightComponent {
        Color color = Color::white();
        float intensity = 1.0f;
    };
} // namespace Axiom