#pragma once
#include "ECS/ECS.h"
#include "SystemManager.h"

namespace Axiom {
    class Scene;

    class PhysicsSystem : public System {
      public:
        PhysicsSystem() = default;
        ~PhysicsSystem() = default;

        void onUpdate(ECS* registry, float deltaTime) override;
    };
} // namespace Axiom
