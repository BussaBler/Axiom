#include "MetalBuffer.h"

namespace Axiom {
    MetalBuffer::MetalBuffer(const CreateInfo& createInfo, MTL::Device* device) : Buffer() {
        bufferSize = createInfo.size;
        metalBuffer = device->newBuffer(bufferSize, axToMetalResourceOptions(createInfo.memoryUsage));
        AX_CORE_ASSERT(metalBuffer, "Failed to create Metal buffer");
    }

    MetalBuffer::~MetalBuffer() {
        if (metalBuffer) {
            metalBuffer->release();
            metalBuffer = nullptr;
        }
    }

    uint32_t MetalBuffer::getSize() const {
        return bufferSize;
    }

    void MetalBuffer::setData(const void* data, uint64_t size, uint64_t offset) {
        AX_CORE_ASSERT(offset + size <= bufferSize, "Data size exceeds buffer capacity");
        std::memcpy(static_cast<uint8_t*>(metalBuffer->contents()) + offset, data, size);
    }
} // namespace Axiom