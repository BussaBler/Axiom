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
        return Entity(entityId, registry.get());
    }

    void Scene::onUpdate(float deltaTime) {
        physicsSystem->onUpdate(registry.get(), deltaTime);
    }

    void Scene::onRender(CommandBuffer* commandBuffer, Texture* renderTarget) {
        auto entities = registry->view<TransformComponent, CameraComponent>();

        for (auto entity : entities) {
            auto& transform = registry->getComponent<TransformComponent>(entity);
            auto& camera = registry->getComponent<CameraComponent>(entity);

            if (camera.isMainCamera) {
                Math::Mat4 projection = camera.camera.getProjection();
                Math::Mat4 view = Math::Mat4::model(transform.position, transform.rotation, transform.scale).inverse();

                renderSystem->onRender(registry.get(), commandBuffer, renderTarget, projection, view);
                return;
            }
        }

        AX_CORE_LOG_WARN("Scene::onRender - No main camera found, skipping render.");
    }

    void Scene::onRender(CommandBuffer* commandBuffer, Texture* renderTarget, const Math::Mat4& projection, const Math::Mat4& view) {
        renderSystem->onRender(registry.get(), commandBuffer, renderTarget, projection, view);
    }
} // namespace Axiom