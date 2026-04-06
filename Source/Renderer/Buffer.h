#pragma once
#include "Utils/BitMaskEnum.h"
#include "axpch.h"

namespace Axiom {
    enum class BufferUsage {
        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Storage = 1 << 2,
        Uniform = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5,
    };

    enum class MemoryUsage {
        None = 0,
        GPUOnly = 1 << 0,
        GPUandCPU = 1 << 1,
    };

    template <> struct EnableBitMaskOperators<BufferUsage> : std::true_type {};
    template <> struct EnableBitMaskOperators<MemoryUsage> : std::true_type {};

    class Buffer {
      public:
        struct CreateInfo {
            uint64_t size = 0;
            BufferUsage usage = BufferUsage::Vertex;
            MemoryUsage memoryUsage = MemoryUsage::GPUOnly;
        };

      public:
        Buffer() = default;
        virtual ~Buffer() = default;

        virtual uint32_t getSize() const = 0;
        virtual void setData(const void* data, uint64_t size, uint64_t offset = 0) = 0;
        template <typename T> void setData(std::span<T> data, uint64_t offset = 0) {
            setData(data.data(), data.size() * sizeof(T), offset);
        }
    };
} // namespace Axiom