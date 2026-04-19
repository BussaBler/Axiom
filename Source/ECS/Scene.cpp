#include "Scene.h"

namespace Axiom {
    Scene::Scene() {
        registry = std::make_unique<Registry>();
        systemManager = std::make_unique<SystemManager>();

        registry->registerComponent<TransformComponent>();
        registry->registerComponent<PhysicsComponent>();
        registry->registerComponent<Sprite2DComponent>();

        physicsSystem = registerSystem<PhysicsSystem, TransformComponent, PhysicsComponent>();
        renderSystem = registerSystem<RenderSystem, TransformComponent, Sprite2DComponent>();
    }

    Entity Scene::createEntity() {
        uint32_t entityId = registry->createEntityId();
        return Entity(entityId, registry.get(), systemManager.get());
    }

    void Scene::onUpdate(float deltaTime) {
        physicsSystem->onUpdate(registry.get(), deltaTime);
    }

    void Scene::onRender(CommandBuffer* commandBuffer, Texture* renderTarget) {
        renderSystem->onRender(registry.get(), commandBuffer, renderTarget);
    }
} // namespace Axiom