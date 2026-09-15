#pragma once

#include "ECS/ECS.h"

#include <cstdint>
#include <tuple>

namespace Axiom {
    class Entity {
      public:
        Entity() : id(0), ecs(nullptr) {}
        Entity(uint32_t id, ECS* ecs) : id(id), ecs(ecs) {}
        ~Entity() = default;

        template <typename T> bool hasComponent() { return ecs->hasComponent<T>(id); }
        bool hasComponent(uint8_t componentId) { return ecs->hasComponent(id, componentId); }
        template <typename T> void addComponent(T component) { ecs->addComponent<T>(id, component); }
        template <typename T> T& getComponent() { return ecs->getComponent<T>(id); }
        template <typename T> const T& getComponent() const { return ecs->getComponent<T>(id); }
        template <typename... Components> std::tuple<Components&...> getComponents() { return ecs->getComponents<Components...>(id); }
        template <typename... Components> std::tuple<const Components&...> getComponents() const { return ecs->getComponents<Components...>(id); }
        std::vector<std::pair<uint8_t, void*>> getAllComponents() { return ecs->getAllComponents(id); }
        template <typename T> void removeComponent() { ecs->removeComponent<T>(id); }
        void* getComponentData(uint8_t componentId) const { return ecs->getComponentData(id, componentId); }

        inline uint32_t getId() const { return id; }

        operator bool() const { return ecs != nullptr; }
        bool operator==(const Entity& other) const { return id == other.id; }

      private:
        uint32_t id;
        ECS* ecs;
    };
} // namespace Axiom
