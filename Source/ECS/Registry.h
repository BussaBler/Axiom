#pragma once
#include "ComponentManager.h"
#include "EntityManager.h"

namespace Axiom {
    class Registry {
      public:
        Registry() {
            entityManager = std::make_unique<EntityManager>();
            componentManager = std::make_unique<ComponentManager>();
        }
        ~Registry() = default;

        [[nodiscard]] uint32_t createEntityId();
        void destroyEntityId(uint32_t entityId);

        template <typename T> void addComponent(uint32_t entityId, T component) {
            componentManager->addComponent<T>(entityId, component);
            entityManager->getComponentSignature(entityId).set(componentManager->getComponentType<T>(), true);
        }
        template <typename T> void removeComponent(uint32_t entityId) {
            componentManager->removeComponent<T>(entityId);
            entityManager->getComponentSignature(entityId).set(componentManager->getComponentType<T>(), false);
        }
        template <typename T> T& getComponent(uint32_t entityId) { return componentManager->getComponent<T>(entityId); }
        std::bitset<32>& getComponentSignature(uint32_t entityId) { return entityManager->getComponentSignature(entityId); }

        template <typename T> void registerComponent() { componentManager->registerComponent<T>(); }
        template <typename T> uint8_t getComponentType() { return componentManager->getComponentType<T>(); }

      private:
        std::unique_ptr<EntityManager> entityManager;
        std::unique_ptr<ComponentManager> componentManager;
    };
} // namespace Axiom