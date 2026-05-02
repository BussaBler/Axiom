#pragma once
#include "Core/Assert.h"
#include "axpch.h"

namespace Axiom {
    const uint32_t MAX_ENTITIES = 5000;

    class EntityManager {
      public:
        EntityManager();
        ~EntityManager() = default;

        uint32_t createEntityId();
        void destroyEntityId(uint32_t entityId);

        std::bitset<32>& getComponentSignature(uint32_t entityId) { return componentSignatures[entityId]; }
        bool isEntityAlive(uint32_t entityId) const { return aliveEntities[entityId]; }

      private:
        std::deque<uint32_t> availableEntitiesIds;
        std::array<bool, MAX_ENTITIES> aliveEntities;
        std::array<std::bitset<32>, MAX_ENTITIES> componentSignatures;
    };
} // namespace Axiom