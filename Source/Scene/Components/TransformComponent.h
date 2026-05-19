#pragma once
#include "Core/Base.h"
#include "Math/AxMath.h"

namespace Axiom {
    struct AX_COMPONENT() TransformComponent {
        AX_PROPERTY() Math::Vec3 position = Math::Vec3::zero();
        AX_PROPERTY() Math::Vec3 rotation = Math::Vec3::zero();
        AX_PROPERTY() Math::Vec3 scale = Math::Vec3::one();
    };
} // namespace Axiom