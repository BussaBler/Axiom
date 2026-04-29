#include "PhysicsSystem.h"

namespace Axiom {
    void PhysicsSystem::onUpdate(Registry* registry, float deltaTime) {
        auto entities = registry->view<TransformComponent, PhysicsComponent>();
    }
} // namespace Axiom