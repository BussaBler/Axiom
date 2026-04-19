#include "EntityManager.h"

namespace Axiom {
    EntityManager::EntityManager() {
        for (uint32_t entityId = 0; entityId < MAX_ENTITIES; ++entityId) {
            availableEntitiesIds.push_back(entityId);
        }
    }

    uint32_t EntityManager::createEntityId() {
        uint32_t entityId = availableEntitiesIds.front();
        availableEntitiesIds.pop_front();
        return entityId;
    }

    void EntityManager::destroyEntityId(uint32_t entityId) {
        componentSignatures[entityId].reset();
        availableEntitiesIds.push_back(entityId);
    }
} // namespace Axiom