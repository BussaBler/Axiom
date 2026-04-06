#include "MetalSemaphore.h"

namespace Axiom {
    MetalSemaphore::MetalSemaphore(MTL::Device* device) {
        event = device->newEvent();
    }

    MetalSemaphore::~MetalSemaphore() {
        if (event) {
            event->release();
            event = nullptr;
        }
    }
} // namespace Axiom