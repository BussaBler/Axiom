#pragma once
#include "MetalUtils.h"
#include "Renderer/Semaphore.h"

namespace Axiom {
    class MetalSemaphore : public Semaphore {
      public:
        MetalSemaphore(MTL::Device* device);
        ~MetalSemaphore() override;

        MTL::Event* getHandle() const {
            return event;
        }

        uint64_t getValue() const {
            return value;
        }
        uint64_t getNextValue() {
            return ++value;
        }

      private:
        MTL::Event* event = nullptr;
        uint64_t value = 0;
    };
} // namespace Axiom