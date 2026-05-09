#pragma once
#include "Core/Base.h"
#include "Math/AxMath.h"

namespace Axiom {
    AX_COMPONENT() struct TransformComponent {
        Math::Vec3 position = Math::Vec3::zero();
        Math::Vec3 rotation = Math::Vec3::zero();
        Math::Vec3 scale = Math::Vec3::one();
    };
} // namespace Axiom