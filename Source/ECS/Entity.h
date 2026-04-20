#pragma once
#include "Registry.h"
#include "SystemManager.h"
#include "axpch.h"

namespace Axiom {
    class Entity {
      public:
        Entity() : id(0), registry(nullptr) {}
        Entity(uint32_t id, Registry* registry) : id(id), registry(registry) {}
        ~Entity() { registry->destroyEntityId(id); }

        template <typename T> void addComponent(T component) { registry->addComponent<T>(id, component); }
        template <typename T> T& getComponent() { return registry->getComponent<T>(id); }
        template <typename T> void removeComponent() { registry->removeComponent<T>(id); }

        inline uint32_t getId() const { return id; }

      private:
        uint32_t id;
        Registry* registry;
    };
} // namespace Axiom