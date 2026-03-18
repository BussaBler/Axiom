#pragma once
#include "ResourceLayout.h"
#include "Buffer.h"
#include "Texture.h"

namespace Axiom {
	class ResourceSet {
	public:
		struct Binding {
			uint32_t binding = 0;
			ResourceType type = ResourceType::UniformBuffer;
			Buffer* buffer = nullptr;
			Texture* texture = nullptr;
		};

	public:
		ResourceSet() = default;
		virtual ~ResourceSet() = default;

		virtual void update(const std::vector<Binding>& bindings) = 0;
	};
}
