#include "Registry.h"

namespace Axiom {
    uint32_t Registry::createEntityId() {
        return entityManager->createEntityId();
    }

    void Registry::destroyEntityId(uint32_t entityId) {
        entityManager->destroyEntityId(entityId);
    }
} // namespace Axiom