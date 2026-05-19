#pragma once
#include "Core/Base.h"
#include "Math/Color.h"

namespace Axiom {
    struct AX_COMPONENT() DirectionalLightComponent {
        AX_PROPERTY() Color color = Color::white();
        AX_PROPERTY() float intensity = 1.0f;
    };
} // namespace Axiom