#pragma once
#include "RenderPass.h"

namespace Axiom {
	class CommandBuffer {
	public:
		CommandBuffer() = default;
		virtual ~CommandBuffer() = default;

		virtual void begin() = 0;
		virtual void end() = 0;
		virtual void beginRendering(const RenderPass& renderPass) = 0;
		virtual void endRendering() = 0;
		virtual void bindPipeline() = 0; // TODO: parameters
		virtual void bindResources() = 0; // TODO: parameters
		virtual void bindVertexBuffers() = 0; // TODO: parameters
		virtual void bindIndexBuffer() = 0; // TODO: parameters
		virtual void draw() = 0; // TODO: parameters
		virtual void drawIndexed() = 0; // TODO: parameters
		virtual void pipelineBarrier(const std::vector<Texture::Barrier>& textureBarries) = 0; // TODO: parameters
	};
}
