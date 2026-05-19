#pragma once
#include "Core/Base.h"
#include "Math/AxMath.h"

namespace Axiom {
    struct AX_COMPONENT() PhysicsComponent {
        AX_PROPERTY() Math::Vec3 velocity = Math::Vec3::zero();
        AX_PROPERTY() Math::Vec3 acceleration = Math::Vec3::zero();
    };
} // namespace Axiom