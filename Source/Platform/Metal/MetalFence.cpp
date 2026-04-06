#include "MetalFence.h"

namespace Axiom {
    MetalFence::MetalFence(MTL::Device *device, bool isSignaled) {
        sharedEvent = device->newSharedEvent();
        if (isSignaled) {
            sharedEvent->setSignaledValue(1);
        } else {
            sharedEvent->setSignaledValue(0);
        }
    }

    MetalFence::~MetalFence() {
        if (sharedEvent) {
            sharedEvent->release();
            sharedEvent = nullptr;
        }
    }

    void MetalFence::wait() {
        sharedEvent->waitUntilSignaledValue(1, std::numeric_limits<uint64_t>::max());
    }

    void MetalFence::reset() {
        sharedEvent->setSignaledValue(0);
    }
} // namespace Axiom