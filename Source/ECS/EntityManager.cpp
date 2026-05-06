#include "EntityManager.h"

namespace Axiom {
    EntityManager::EntityManager() {
        for (uint32_t entityId = 1; entityId < MAX_ENTITIES + 1; ++entityId) {
            availableEntitiesIds.push_back(entityId);
            aliveEntities[entityId] = false;
        }
    }

    uint32_t EntityManager::createEntityId() {
        uint32_t entityId = availableEntitiesIds.front();
        availableEntitiesIds.pop_front();
        aliveEntities[entityId] = true;
        return entityId;
    }

    void EntityManager::destroyEntityId(uint32_t entityId) {
        componentSignatures[entityId].reset();
        aliveEntities[entityId] = false;
        availableEntitiesIds.push_back(entityId);
    }
} // namespace Axiom