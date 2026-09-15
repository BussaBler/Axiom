#include "axpch.h"

#include "ECS.h"

#include "ECS/Entity.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace Axiom {
    ECS::ECS() {
        for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
            availableEntitiesId.push_back(i);
        }
    }

    Entity ECS::newEntity() {
        uint32_t entityId = availableEntitiesId.front();
        availableEntitiesId.pop_front();
        activeEntityCount++;
        return Entity(entityId, this);
    }

    void ECS::deleteEntity(Entity entity) {
        for (uint32_t i = 0; i < MAX_COMPONENTS; i++) {
            if (entitiesComponentSignature[entity.getId()].test(i)) {
                componentsArray[i]->onEntityDelete(entity.getId());
            }
        }

        entitiesComponentSignature[entity.getId()].reset();
        activeEntityCount--;
        availableEntitiesId.push_back(entity.getId());
    }

    std::vector<std::pair<uint8_t, void*>> ECS::getAllComponents(uint32_t entityId) {
        std::vector<std::pair<uint8_t, void*>> components;
        for (uint32_t i = 0; i < MAX_COMPONENTS; i++) {
            if (entitiesComponentSignature[entityId].test(i)) {
                void* data = componentsArray[i]->getComponentData(entityId);
                if (data) {
                    components.push_back({i, data});
                }
            }
        }

        return components;
    }
} // namespace Axiom
