#pragma once
#include "Core/Base.h"
#include "Math/AxMath.h"

namespace Axiom {
    AX_COMPONENT() struct PhysicsComponent {
        Math::Vec3 velocity = Math::Vec3::zero();
        Math::Vec3 acceleration = Math::Vec3::zero();
    };
} // namespace Axiom