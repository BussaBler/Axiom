#pragma once
#include "Device.h"
#include "RenderPass.h"

namespace Axiom {
	struct PipelineConfigInfo;

	enum class PipelineBindPoint {
		GRAPHICS = 0,
		COMPUTE = 1
	};

	class Pipeline {
	public:
		Pipeline() = default;
		virtual ~Pipeline() = default;
		virtual void bind(CommandBuffer& commandBuffer, PipelineBindPoint bindPoint) = 0;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<Pipeline> create(Device& deviceRef, RenderPass& renderPassRef, const PipelineConfigInfo& configInfo);

	protected:
		std::any handle;
	};
}

