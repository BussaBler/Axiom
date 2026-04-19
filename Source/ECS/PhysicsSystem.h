#pragma once
#include "Component.h"
#include "SystemManager.h"

namespace Axiom {
    class Scene;

    class PhysicsSystem : public System {
      public:
        PhysicsSystem() = default;
        ~PhysicsSystem() = default;

        void onUpdate(Registry* registry, float deltaTime) override;
    };
} // namespace Axiom