#pragma once
#include "axpch.h"

namespace Axiom {
	enum class BufferUsage : uint8_t {
		Vertex = 1 << 0,
		Index = 1 << 1,
		Storage = 1 << 2,
		Uniform = 1 << 3,
		TransferSrc = 1 << 4,
		TransferDst = 1 << 5,
	};

	inline uint8_t operator&(BufferUsage left, BufferUsage right) {
		return static_cast<uint8_t>(left) & static_cast<uint8_t>(right);
	}

	enum class MemoryUsage {
		GPUOnly = 1 << 0,
		GPUandCPU = 1 << 1,
	};

	inline uint8_t operator&(MemoryUsage left, MemoryUsage right) {
		return static_cast<uint8_t>(left) & static_cast<uint8_t>(right);
	}

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
		virtual void* map() = 0;
		virtual void unmap() = 0;

	};
}