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

        Entity createEntity(const std::string& name = "Entity");
        Entity getEntity(uint32_t entityId);
        Entity getEntity(const std::string& name);

        template <typename... Components> View view() { return registry->view<Components...>(); }

        void onUpdate(float deltaTime);
        void onRender(CommandBuffer* commandBuffer, Texture* renderTarget);
        void onRender(CommandBuffer* commandBuffer, Texture* renderTarget, const Math::Mat4& projection, const Math::Mat4& view);

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