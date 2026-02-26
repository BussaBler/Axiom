#pragma once
#include "Math/AxMath.h"
#include "CommandBuffer.h"
#include "Swapchain.h"

namespace Axiom {
	struct RenderPassCreateInfo {
		Math::Vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		float clearDepth = 1.0f;
		uint32_t clearStencil = 0;

		bool operator==(const RenderPassCreateInfo& other) const {
			return clearStencil == other.clearStencil && clearColor == other.clearColor;
		}
	};

	struct RenderPassToken;

	class RenderPassCache {
		friend class RenderPassToken;
	public:
		virtual ~RenderPassCache() = default;

		// Get or create (on miss) a render pass based on specific parameters
		virtual RenderPassToken get(RenderPassCreateInfo renderPassCreateInfo) = 0;
		virtual void updateSwapchain(Swapchain& swapchain) = 0;

	protected:
		virtual void begin(RenderPassToken& token, CommandBuffer& commandBuffer) = 0;
		virtual void end(RenderPassToken& token, CommandBuffer& commandBuffer) = 0;
		virtual void* get(RenderPassToken& token) = 0;
	};

	struct RenderPassToken {
		RenderPassToken(uint32_t index, uint32_t generation, RenderPassCache* owner) : index(index), generation(generation), owner(owner) {}

		void begin(CommandBuffer& commandBuffer) {
			owner->begin(*this, commandBuffer);
		}
		void end(CommandBuffer& commandBuffer) {
			owner->end(*this, commandBuffer);
		}
		void* get() {
			return owner->get(*this);
		}

		uint32_t getIndex() const { return index; }
		uint32_t getGeneration() const { return generation; }

	private:
		RenderPassCache* owner = nullptr;
		uint32_t index = 0;
		uint32_t generation = 0;
	};
}

template <>
struct std::hash<Axiom::RenderPassCreateInfo> {
	std::size_t operator()(const Axiom::RenderPassCreateInfo& key) const noexcept {
		auto h1 = std::hash<int>{}(key.clearColor.x());
		auto h2 = std::hash<int>{}(key.clearColor.y());
		auto h3 = std::hash<uint32_t>{}(key.clearStencil);

		return h1 ^ ((h2 << 4) ^ h3 << 3);
	}
};
