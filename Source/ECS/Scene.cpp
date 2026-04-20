#include "Scene.h"

namespace Axiom {
    Scene::Scene() {
        registry = std::make_unique<Registry>();
        systemManager = std::make_unique<SystemManager>();

        registry->registerComponent<TagComponent>();
        registry->registerComponent<TransformComponent>();
        registry->registerComponent<PhysicsComponent>();
        registry->registerComponent<Sprite2DComponent>();
        registry->registerComponent<CameraComponent>();

        physicsSystem = registerSystem<PhysicsSystem, TransformComponent, PhysicsComponent>();
        renderSystem = registerSystem<RenderSystem, TransformComponent, Sprite2DComponent>();
    }

    Entity Scene::createEntity(const std::string& name) {
        uint32_t entityId = registry->createEntityId();
        registry->addComponent<TagComponent>(entityId, TagComponent{name});
        return Entity(entityId, registry.get());
    }

    Entity Scene::getEntity(uint32_t entityId) {
        return Entity(entityId, registry.get());
    }

    Entity Scene::getEntity(const std::string& name) {
        auto entities = registry->view<TagComponent>();

        for (auto entity : entities) {
            auto& tag = registry->getComponent<TagComponent>(entity);
            if (tag.tag == name) {
                return Entity(entity, registry.get());
            }
        }
        return Entity(0, nullptr);
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