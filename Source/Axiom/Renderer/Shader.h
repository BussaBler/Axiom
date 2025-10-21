#pragma once

namespace Axiom {
	class CommandBuffer;
	class Resource;
	class Texture;

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void bind(CommandBuffer& commandBuffer) const = 0;
		virtual void updateFrameIndex(uint32_t frameIndex) {}
		virtual void bindDescriptors(CommandBuffer& commandBuffer) {};
		// The buffer must contain the data for all descriptor sets used by this shader
		virtual void bindUniformBuffer(Resource& uniformBuffer) {};
		virtual void bindPushConstants(CommandBuffer& commandBuffer, const void* data, uint32_t size, uint32_t offset = 0) const {}
		virtual void bindTexture(Texture& texture) {}
	};
}
