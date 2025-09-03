#pragma once
#include "CommandBuffer.h"
#include "Framebuffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "Math/AxMath.h"

namespace Axiom {
	class RenderPass {
	public:
		RenderPass() = default;
		virtual ~RenderPass() = default;

		virtual void begin(CommandBuffer& commandBuffer, Framebuffer& framebuffer) const = 0;
		virtual void end(CommandBuffer& commandBuffer) const = 0;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<RenderPass> create(Device& deviceRef, SwapChain& swapChainRef, Math::Vec2 offset, Math::Vec2 extent, Math::Vec4 clearColor, float depth, uint32_t stencil);

	protected:
		std::any handle;
	};
}

