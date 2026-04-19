#pragma once
#include "EntityManager.h"

namespace Axiom {
    class IComponentArray {
      public:
        virtual ~IComponentArray() = default;
        virtual void entityDestroyed(uint32_t entityId) = 0;
    };

    template <typename T> class ComponentArray : public IComponentArray {
      public:
        ComponentArray() = default;
        ~ComponentArray() = default;

        void insertData(uint32_t entityId, T component) {
            AX_CORE_ASSERT(entityToIndexMap.find(entityId) == entityToIndexMap.end(), "Component added to same entity more than once.");

            size_t newIndex = size;
            entityToIndexMap[entityId] = newIndex;
            indexToEntityMap[newIndex] = entityId;
            componentArray[newIndex] = component;
            size++;
        }
        void removeData(uint32_t entityId) {
            AX_CORE_ASSERT(entityToIndexMap.find(entityId) != entityToIndexMap.end(), "Trying to remove non-existent component.");

            size_t indexOfRemovedEntity = entityToIndexMap[entityId];
            size_t indexOfLastElement = size - 1;
            componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

            uint32_t entityOfLastElement = indexToEntityMap[indexOfLastElement];
            entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
            indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

            entityToIndexMap.erase(entityId);
            indexToEntityMap.erase(indexOfLastElement);

            size--;
        }
        T& getData(uint32_t entityId) {
            AX_CORE_ASSERT(entityToIndexMap.find(entityId) != entityToIndexMap.end(), "Trying to access non-existent component.");

            return componentArray[entityToIndexMap[entityId]];
        }
        void entityDestroyed(uint32_t entityId) override {
            if (entityToIndexMap.find(entityId) != entityToIndexMap.end()) {
                removeData(entityId);
            }
        }

      private:
        std::array<T, MAX_ENTITIES> componentArray;
        std::unordered_map<uint32_t, size_t> entityToIndexMap;
        std::unordered_map<size_t, uint32_t> indexToEntityMap;
        size_t size = 0;
    };

    class ComponentManager {
      public:
        ComponentManager() = default;
        ~ComponentManager() = default;

        template <typename T> void registerComponent() {
            const char* typeName = typeid(T).name();
            AX_CORE_ASSERT(componentTypes.find(typeName) == componentTypes.end(), "Registering component type more than once.");

            componentTypes.insert({typeName, nextComponentType++});
            componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
        }

        template <typename T> uint8_t getComponentType() {
            const char* typeName = typeid(T).name();
            AX_CORE_ASSERT(componentTypes.find(typeName) != componentTypes.end(), "Component not registered before use.");

            return componentTypes[typeName];
        }

        template <typename T> void addComponent(uint32_t entityId, T component) { getComponentArray<T>()->insertData(entityId, component); }
        template <typename T> void removeComponent(uint32_t entityId) { getComponentArray<T>()->removeData(entityId); }
        template <typename T> T& getComponent(uint32_t entityId) { return getComponentArray<T>()->getData(entityId); }
        void entityDestroyed(uint32_t entityId) {
            for (auto const& pair : componentArrays) {
                auto const& component = pair.second;
                component->entityDestroyed(entityId);
            }
        }

      private:
        template <typename T> std::shared_ptr<ComponentArray<T>> getComponentArray() {
            const char* typeName = typeid(T).name();
            AX_CORE_ASSERT(componentTypes.find(typeName) != componentTypes.end(), "Component not registered before use.");

            return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
        }

      private:
        std::unordered_map<const char*, uint8_t> componentTypes;
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;
        uint8_t nextComponentType = 0;
    };
} // namespace Axiom