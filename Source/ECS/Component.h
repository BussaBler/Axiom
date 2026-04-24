#pragma once
#include "Asset/UUID.h"
#include "Core/Reflections/TypeRegistry.h"
#include "Math/AxMath.h"
#include "Math/Color.h"
#include "Renderer/Camera.h"
#include "axpch.h"

namespace Axiom {
    struct TagComponent {
        std::string tag;
    };
    AX_REGISTER_FIELD(TagComponent, tag, FieldType::String);

    struct TransformComponent {
        Math::Vec3 position = Math::Vec3::zero();
        Math::Vec3 rotation = Math::Vec3::zero();
        Math::Vec3 scale = Math::Vec3::one();
    };
    AX_REGISTER_FIELD(TransformComponent, position, FieldType::Vec3);
    AX_REGISTER_FIELD(TransformComponent, rotation, FieldType::Vec3);
    AX_REGISTER_FIELD(TransformComponent, scale, FieldType::Vec3);

    struct PhysicsComponent {
        Math::Vec3 velocity = Math::Vec3::zero();
        Math::Vec3 acceleration = Math::Vec3::zero();
    };
    AX_REGISTER_FIELD(PhysicsComponent, velocity, FieldType::Vec3);
    AX_REGISTER_FIELD(PhysicsComponent, acceleration, FieldType::Vec3);

    struct CameraComponent {
        Camera camera;
        bool isMainCamera = false;
        bool isFixedAspectRatio = false;
    };
    // AX_REGISTER_FIELD(CameraComponent, camera, FieldType::AssetHandle);
    AX_REGISTER_FIELD(CameraComponent, isMainCamera, FieldType::Bool);
    AX_REGISTER_FIELD(CameraComponent, isFixedAspectRatio, FieldType::Bool);

    struct Sprite2DComponent {
        UUID textureId;
        Color color = Color::white();
    };
    AX_REGISTER_FIELD(Sprite2DComponent, textureId, FieldType::AssetHandle);
    AX_REGISTER_FIELD(Sprite2DComponent, color, FieldType::Color);

    struct MeshComponent {
        UUID meshId;
    };
    AX_REGISTER_FIELD(MeshComponent, meshId, FieldType::AssetHandle);

    struct DirectionalLightComponent {
        Color color = Color::white();
        float intensity = 1.0f;
    };
    AX_REGISTER_FIELD(DirectionalLightComponent, color, FieldType::Color);
    AX_REGISTER_FIELD(DirectionalLightComponent, intensity, FieldType::Float);

} // namespace Axiom