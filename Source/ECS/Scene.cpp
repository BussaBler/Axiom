#include "Scene.h"

namespace Axiom {
    Scene::Scene() {
        registry = std::make_unique<Registry>();
        systemManager = std::make_unique<SystemManager>();

        registry->registerComponent<TagComponent>();
        registry->registerComponent<TransformComponent>();
        registry->registerComponent<PhysicsComponent>();
        registry->registerComponent<Sprite2DComponent>();
        registry->registerComponent<MeshComponent>();
        registry->registerComponent<CameraComponent>();
        registry->registerComponent<DirectionalLightComponent>();

        physicsSystem = registerSystem<PhysicsSystem, TransformComponent, PhysicsComponent>();
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
} // namespace Axiom