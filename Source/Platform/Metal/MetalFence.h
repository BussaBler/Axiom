#pragma once
#include "MetalUtils.h"
#include "Renderer/Fence.h"

namespace Axiom {
    class MetalFence : public Fence {
      public:
        MetalFence(MTL::Device* device, bool isSignaled);
        ~MetalFence() override;

        void wait() override;
        void reset() override;

        MTL::SharedEvent* getSharedEvent() const {
            return sharedEvent;
        }

      private:
        MTL::SharedEvent* sharedEvent = nullptr;
    };
} // namespace Axiom