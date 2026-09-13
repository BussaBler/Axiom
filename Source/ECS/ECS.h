#pragma once

#include "Core/Assert.h"

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>

namespace Axiom {
    constexpr uint32_t MAX_ENTITIES = 5000;

    class Entity;
    class ECS;
    template <typename First, typename... Rest> class View;

    class IComponentArray {
      public:
        virtual ~IComponentArray() = default;

        virtual void onEntityDelete(uint32_t entityId) = 0;
        virtual void* getComponentData(uint32_t entityId) = 0;
    };

    template <typename T> class ComponentArray : public IComponentArray {
      public:
        ComponentArray() { sparseEntityToIndex.fill(std::numeric_limits<size_t>::max()); }
        ~ComponentArray() = default;

        void onEntityDelete(uint32_t entityId) override {
            if (sparseEntityToIndex[entityId] != std::numeric_limits<size_t>::max()) {
                removeData(entityId);
            }
        }
        void* getComponentData(uint32_t entityId) override {
            if (sparseEntityToIndex[entityId] != std::numeric_limits<size_t>::max()) {
                return &componentArray[sparseEntityToIndex[entityId]];
            }
            return nullptr;
        }

        void insertData(uint32_t entityId, T component) {
            if (sparseEntityToIndex[entityId] != std::numeric_limits<size_t>::max()) {
                componentArray[sparseEntityToIndex[entityId]] = component;
                return;
            }
            uint32_t newIndex = size;
            sparseEntityToIndex[entityId] = newIndex;
            indexToEntity[newIndex] = entityId;
            componentArray[newIndex] = component;
            size++;
        }
        void removeData(uint32_t entityId) {
            AX_CORE_ASSERT(sparseEntityToIndex[entityId] != std::numeric_limits<size_t>::max(), "Cannot remove non-existent component data from entity");

            size_t removeDataIndex = sparseEntityToIndex[entityId];
            size_t indexOfLastElement = size - 1;

            componentArray[removeDataIndex] = componentArray[indexOfLastElement];
            // who the last component belonges to
            uint32_t entityOfLastElement = indexToEntity[indexOfLastElement];
            // update the sparse array so the moved entity points to its new dense index
            sparseEntityToIndex[entityOfLastElement] = removeDataIndex;
            // update the dense reverse-lookup array to reflect the new owner of this slot
            indexToEntity[removeDataIndex] = entityOfLastElement;
            sparseEntityToIndex[entityId] = std::numeric_limits<size_t>::max();
            size--;
        }

        T& getData(uint32_t entityId) { return componentArray[sparseEntityToIndex[entityId]]; }
        const T& getData(uint32_t entityId) const { return componentArray[sparseEntityToIndex[entityId]]; }
        inline size_t getSize() const { return size; }
        uint32_t getEntityIdAtIndex(uint32_t index) const { return indexToEntity[index]; }

      private:
        std::array<T, MAX_ENTITIES> componentArray;
        std::array<uint32_t, MAX_ENTITIES> indexToEntity;
        std::array<size_t, MAX_ENTITIES> sparseEntityToIndex;
        size_t size = 0;
    };

    class ECS {
        friend class Entity;

      public:
        ECS();
        ~ECS() = default;

        [[nodiscard]] Entity newEntity();
        void deleteEntity(Entity entity);

        template <typename T> inline static uint8_t getComponentId() {
            static uint8_t componentId = getNextComponentId();
            return componentId;
        }
        template <typename T> void registerComponent() {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Cannot register any more components");
            componentsArray[componentId] = std::make_unique<ComponentArray<T>>();
        }

        template <typename T> void addComponent(uint32_t entityId, T component) {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to add unregistered component");

            ComponentArray<T>* pComponentArray = static_cast<ComponentArray<T>*>(componentsArray[componentId].get());
            pComponentArray->insertData(entityId, component);
            entitiesComponentSignature[entityId].set(componentId, true);
        }
        template <typename T> void removeComponent(uint32_t entityId) {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to remove unregistered component");

            ComponentArray<T>* pComponentArray = static_cast<ComponentArray<T>*>(componentsArray[componentId].get());
            pComponentArray->removeData(entityId);
            entitiesComponentSignature[entityId].set(componentId, false);
        }
        template <typename T> bool hasComponent(uint32_t entityId) {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            return entitiesComponentSignature[entityId].test(componentId);
        }
        bool hasComponent(uint32_t entityId, uint8_t componentId) {
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            return entitiesComponentSignature[entityId].test(componentId);
        }
        template <typename T> T& getComponent(uint32_t entityId) {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to get unregistered component");

            ComponentArray<T>* pComponentArray = static_cast<ComponentArray<T>*>(componentsArray[componentId].get());
            return pComponentArray->getData(entityId);
        }
        template <typename T> const T& getComponent(uint32_t entityId) const {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to get unregistered component");

            ComponentArray<T>* pComponentArray = static_cast<ComponentArray<T>*>(componentsArray[componentId].get());
            return pComponentArray->getData(entityId);
        }
        template <typename T> void* getComponentData(uint32_t entityId) {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to get data for an unregistered component");

            if (hasComponent<T>(entityId)) {
                return componentsArray[componentId]->getComponentData(entityId);
            }
            return nullptr;
        }
        void* getComponentData(uint32_t entityId, uint8_t componentId) {
            AX_CORE_ASSERT(componentId < componentsArray.size(), "Component out of bounds for this ECS");
            AX_CORE_ASSERT(componentsArray[componentId], "Trying to get data for an unregistered component");

            if (hasComponent(entityId, componentId)) {
                return componentsArray[componentId]->getComponentData(entityId);
            }
            return nullptr;
        }
        template <typename T> ComponentArray<T>* getComponentArray() {
            uint8_t componentId = getComponentId<T>();
            AX_CORE_ASSERT(componentsArray[componentId], "Fetching unregistered component array");
            return static_cast<ComponentArray<T>*>(componentsArray[componentId].get());
        }
        const std::bitset<32>& getComponentSignature(uint32_t entityId) const { return entitiesComponentSignature[entityId]; }

        std::vector<std::pair<uint8_t, void*>> getComponents(uint32_t entityId);
        template <typename First, typename... Rest> View<First, Rest...> view();

      private:
        inline static uint8_t getNextComponentId() {
            static uint8_t lasId = 0;
            return lasId++;
        }

      private:
        static constexpr size_t MAX_COMPONENTS = 32;

        uint32_t activeEntityCount = 0;
        std::deque<uint32_t> availableEntitiesId;
        std::array<std::bitset<32>, MAX_ENTITIES> entitiesComponentSignature;
        std::array<std::unique_ptr<IComponentArray>, MAX_COMPONENTS> componentsArray;
    };

    template <typename First, typename... Rest> class View {
      public:
        View(ECS* ecs) : ecs(ecs) {
            signature.set(ECS::getComponentId<First>());
            ((signature.set(ECS::getComponentId<Rest>())), ...);

            leadArray = ecs->getComponentArray<First>();
        }
        ~View() = default;

        struct Iterator {
            Iterator(ECS* ecs, std::bitset<32> signature, size_t index, ComponentArray<First>* leadArray)
                : ecs(ecs), signature(signature), index(index), leadArray(leadArray) {
                advanceToValid();
            }

            uint32_t operator*() const { return leadArray->getEntityIdAtIndex(index); }

            Iterator& operator++() {
                index++;
                advanceToValid();
                return *this;
            }

            bool operator!=(const Iterator& other) const { return index != other.index; }

          private:
            ECS* ecs;
            std::bitset<32> signature;
            size_t index;
            ComponentArray<First>* leadArray;

            void advanceToValid() {
                while (index < leadArray->getSize()) {
                    uint32_t entityId = leadArray->getEntityIdAtIndex(index);
                    if ((ecs->getComponentSignature(entityId) & signature) == signature) {
                        break;
                    }
                    index++;
                }
            }
        };

        Iterator begin() const { return Iterator(ecs, signature, 0, leadArray); }
        Iterator end() const { return Iterator(ecs, signature, leadArray->getSize(), leadArray); }

      private:
        ECS* ecs;
        std::bitset<32> signature;
        ComponentArray<First>* leadArray;
    };

    template <typename First, typename... Rest> inline View<First, Rest...> ECS::view() {
        return View<First, Rest...>(this);
    }
} // namespace Axiom
