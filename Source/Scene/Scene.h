#pragma once
#include "ECS/ECS.h"
#include "ECS/Entity.h"
#include "ECS/PhysicsSystem.h"

namespace Axiom {
    class Scene {
        friend class SceneSerializer;

      public:
        Scene();
        ~Scene() = default;

        Entity newEntity(const std::string& name = "Entity");
        void deleteEntity(Entity entity);
        Entity getEntity(uint32_t entityId);
        Entity getEntity(const std::string& name);

        template <typename First, typename... Rest> View<First, Rest...> view() { return ecs->view<First, Rest...>(); }

        void onUpdate(float deltaTime);

        // TODO: maybe make register component and system public so the scene doesn't
        // have to keep track of them? Or maybe make a scene builder that handles all of this?

      private:
        template <typename S, typename... RequiredComponents> [[nodiscard]] std::shared_ptr<S> registerSystem() {
            std::shared_ptr<S> system = systemManager->registerSystem<S>();
            std::bitset<32> signature;
            ((signature.set(ECS::getComponentId<RequiredComponents>())), ...);

            systemManager->setSignature<S>(signature);
            return system;
        }

      private:
        std::unique_ptr<ECS> ecs;
        std::unique_ptr<SystemManager> systemManager;
        std::shared_ptr<PhysicsSystem> physicsSystem;
    };
} // namespace Axiom
