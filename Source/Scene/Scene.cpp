#include "axpch.h"

#include "Scene.h"

#include "ECS/ComponentRegistry.h"
#include "ECS/Components/PhysicsComponent.h"
#include "ECS/Components/TagComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/ECS.h"

namespace Axiom {
    Scene::Scene() {
        ecs = std::make_unique<ECS>();
        systemManager = std::make_unique<SystemManager>();

        for (const auto& registerFn : ComponentRegistry::getRegisterFunctions()) {
            registerFn(ecs.get());
        }

        physicsSystem = registerSystem<PhysicsSystem, TransformComponent, PhysicsComponent>();
    }

    Entity Scene::newEntity(const std::string& name) {
        Entity entity = ecs->newEntity();
        entity.addComponent<TagComponent>({name});
        return entity;
    }

    void Scene::deleteEntity(Entity entity) {
        ecs->deleteEntity(entity);
    }

    Entity Scene::getEntity(uint32_t entityId) {
        return Entity(entityId, ecs.get());
    }

    Entity Scene::getEntity(const std::string& name) {
        auto entities = ecs->view<TagComponent>();

        for (auto entity : entities) {
            const auto& tag = ecs->getComponent<TagComponent>(entity);
            if (tag.tag == name) {
                return Entity(entity, ecs.get());
            }
        }
        return Entity(0, nullptr);
    }

    void Scene::onUpdate(float deltaTime) {
        physicsSystem->onUpdate(ecs.get(), deltaTime);
    }
} // namespace Axiom
