#pragma once
#include "Registry.h"
#include "SystemManager.h"
#include "axpch.h"

namespace Axiom {
    class Entity {
      public:
        Entity() : id(0), registry(nullptr), systemManager(nullptr) {}
        Entity(uint32_t id, Registry* registry, SystemManager* systemManager) : id(id), registry(registry), systemManager(systemManager) {}
        ~Entity() {
            registry->destroyEntityId(id);
            systemManager->entityDestroyed(id);
        }

        template <typename T> void addComponent(T component) {
            registry->addComponent<T>(id, component);
            systemManager->entitySignatureChanged(id, registry->getComponentSignature(id));
        }
        template <typename T> T& getComponent() { return registry->getComponent<T>(id); }
        template <typename T> void removeComponent() {
            registry->removeComponent<T>(id);
            systemManager->entitySignatureChanged(id, registry->getComponentSignature(id));
        }

        inline uint32_t getId() const { return id; }

      private:
        uint32_t id;
        Registry* registry;
        SystemManager* systemManager;
    };
} // namespace Axiom