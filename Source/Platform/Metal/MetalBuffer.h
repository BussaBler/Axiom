#pragma once
#include "MetalUtils.h"
#include "Renderer/Buffer.h"

namespace Axiom {
    class MetalBuffer : public Buffer {
      public:
        MetalBuffer(const CreateInfo &createInfo, MTL::Device *device);
        ~MetalBuffer() override;

        uint32_t getSize() const override;
        void setData(const void *data, uint64_t size, uint64_t offset = 0) override;

        MTL::Buffer *getHandle() const {
            return metalBuffer;
        }

      private:
        MTL::Buffer *metalBuffer = nullptr;
        uint32_t bufferSize = 0;
    };
} // namespace Axiom