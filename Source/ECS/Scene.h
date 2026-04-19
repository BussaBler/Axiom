#pragma once
#include "Entity.h"
#include "PhysicsSystem.h"
#include "Registry.h"
#include "RenderSystem.h"
#include "SystemManager.h"

namespace Axiom {
    class Scene {
      public:
        Scene();
        ~Scene() = default;

        Entity createEntity();

        void onUpdate(float deltaTime);
        void onRender(CommandBuffer* commandBuffer, Texture* renderTarget);

        // TODO: maybe make register component and system public so the scene doesn't
        // have to keep track of them? Or maybe make a scene builder that handles all of this?

      private:
        template <typename S, typename... RequiredComponents> [[nodiscard]] std::shared_ptr<S> registerSystem() {
            std::shared_ptr<S> system = systemManager->registerSystem<S>();
            std::bitset<32> signature;
            ((signature.set(registry->getComponentType<RequiredComponents>())), ...);
            systemManager->setSignature<S>(signature);
            return system;
        }

      private:
        std::unique_ptr<Registry> registry;
        std::unique_ptr<SystemManager> systemManager;
        std::shared_ptr<PhysicsSystem> physicsSystem;
        std::shared_ptr<RenderSystem> renderSystem;
    };
} // namespace Axiom