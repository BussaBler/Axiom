#include "axpch.h"

#include "PhysicsSystem.h"

#include "ECS/Components/PhysicsComponent.h"
#include "ECS/Components/TransformComponent.h"

namespace Axiom {
    void PhysicsSystem::onUpdate(ECS* registry, float deltaTime) {
        auto entities = registry->view<TransformComponent, PhysicsComponent>();
    }
} // namespace Axiom
