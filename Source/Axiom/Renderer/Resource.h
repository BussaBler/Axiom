#pragma once
#include "axpch.h"
#include "ResourceView.h"

namespace Axiom {
	enum ResourceType {
		Buffer = 0,
		Image = 1,
	};

	enum ResourceFormat {
		Unknown = 0,
		R8_U = 1,
		RG8_U = 2,
		RGBA8_U = 3,
		RGBA8_S = 4,
		BRGA8_U = 5,
		BRGA8_S = 6,
		RGBA16_F = 7,
		RGBA32_F = 8,
		D24S8 = 9,
		D32_F = 10,
		D32S8 = 11
	};

	enum ResourceUsage {
		VertexBuffer = 1 << 0,
		IndexBuffer = 1 << 1,
		RenderTarget = 1 << 2,
		ShaderResource = 1 << 3,
		UniformBuffer = 1 << 4,
		TransferDst = 1 << 5,
		TransferSrc = 1 << 6,
		CopyDst = 1 << 7,
		CopySrc = 1 << 8,
		DepthStencil = 1 << 9,
	};



	enum ResourceAspectMask {
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2
	};

	enum ResourceMemoryUsage {
		GPU_Only = 1 << 0,
		CPU_Only = 1 << 1,
		CPU_To_GPU = 1 << 2,
		GPU_To_CPU = 1 << 3
	};

	struct ResourceCreateInfo {
		uint32_t width = 1;
		uint32_t height = 1;
		uint64_t size = 0;
		ResourceFormat format = ResourceFormat::Unknown;
		uint32_t usage = ResourceUsage::ShaderResource;
		ResourceType type = ResourceType::Buffer;
		uint32_t memoryUsage = ResourceMemoryUsage::GPU_Only;
		uint32_t aspectMask = ResourceAspectMask::Color;
	};

	struct ResourceViewCreateInfo {
		ResourceFormat format = ResourceFormat::Unknown;
		uint32_t aspectMask = ResourceAspectMask::Color;
	};

	class Resource {
	public:
		virtual ~Resource() = default;
		virtual void loadData(void* data, uint64_t size, uint64_t offset = 0, uint32_t flags = 0) = 0;
		virtual void copyTo(Resource& destination, uint64_t srcOffset = 0, uint64_t dstOffset = 0, uint64_t size = UINT64_MAX) {}

		//virtual ResourceView& getView(const ResourceViewCreateInfo& resourceViewCreateInfo) = 0;
		virtual uint32_t generateId() = 0;

	protected:
		ResourceCreateInfo createInfo;
	};
}
