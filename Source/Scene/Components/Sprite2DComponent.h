#pragma once
#include "Asset/UUID.h"
#include "Component.h"
#include "Math/Color.h"

namespace Axiom {
    AX_COMPONENT struct Sprite2DComponent {
        UUID textureId;
        Color color = Color::white();
    };
} // namespace Axiom