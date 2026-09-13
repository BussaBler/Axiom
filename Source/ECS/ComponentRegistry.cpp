#include "axpch.h"

#include "ComponentRegistry.h"

#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/DirectionalLightComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/PhysicsComponent.h"
#include "ECS/Components/Sprite2DComponent.h"
#include "ECS/Components/TagComponent.h"
#include "ECS/Components/TransformComponent.h"

namespace Axiom {
    void ComponentRegistry::registerDefaultComponents() {
        registerComponent<CameraComponent>();
        registerComponent<DirectionalLightComponent>();
        registerComponent<MeshComponent>();
        registerComponent<PhysicsComponent>();
        registerComponent<Sprite2DComponent>();
        registerComponent<TagComponent>();
        registerComponent<TransformComponent>();
    }
} // namespace Axiom
