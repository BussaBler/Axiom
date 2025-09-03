#pragma once
#include "Device.h"
#include "RenderPass.h"
#include "CommandBuffer.h"

namespace Axiom {
	class Shader {
	public:
		Shader() = default;
		virtual ~Shader() = default;

		virtual void use(CommandBuffer& commandBuffer) = 0;
		virtual void createPipeline(RenderPass& renderPass, uint32_t width, uint32_t height) = 0;
	};
}

