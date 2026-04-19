#pragma once
#include "Asset/UUID.h"
#include "Math/AxMath.h"
#include "Math/Color.h"
#include "axpch.h"

namespace Axiom {
    struct TransformComponent {
        Math::Vec3 position = Math::Vec3::zero();
        Math::Vec3 rotation = Math::Vec3::zero();
        Math::Vec3 scale = Math::Vec3::one();
    };

    struct PhysicsComponent {
        Math::Vec3 velocity = Math::Vec3::zero();
        Math::Vec3 acceleration = Math::Vec3::zero();
    };

    struct CameraComponent {
        Math::Mat4 projection = Math::Mat4::identity();
        bool isMainCamera = false;
    };

    struct Sprite2DComponent {
        UUID textureId;
        Color color = Color::white();
    };

} // namespace Axiom