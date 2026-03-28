#pragma once
#include "axpch.h"
#include "Utils/BitMaskEnum.h"

namespace Axiom {
	enum class ResourceType {
		UniformBuffer,
		StorageBuffer,
		Texture,
		Sampler,
		CombinedTextureSampler,
	};

	enum class ShaderStage {
		None = 0,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Compute = 1 << 2,
	};

	template<>
	struct EnableBitMaskOperators<ShaderStage> : std::true_type {};

	class ResourceLayout {
	public:
		ResourceLayout() = default;
		virtual ~ResourceLayout() = default;

	public:
		struct BindingCreateInfo {
			uint32_t binding = 0;
			ResourceType type = ResourceType::UniformBuffer;
			ShaderStage stages = ShaderStage::None;
			uint32_t count = 1;
		};
	};
}
