#include "PhysicsSystem.h"

namespace Axiom {
    void PhysicsSystem::onUpdate(Registry* registry, float deltaTime) {
        for (const auto& entity : entities) {
            auto& physicComponent = registry->getComponent<TransformComponent>(entity);
        }
    }
} // namespace Axiom