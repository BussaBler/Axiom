#pragma once

namespace Axiom {
	class CommandBuffer;
	class Resource;

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void bind(CommandBuffer& commandBuffer) const = 0;
		virtual void bindDescriptors(CommandBuffer& commandBuffer, uint32_t offset = 0) const {};
		virtual void bindUniformBuffer(Resource& uniformBuffer) {};
	};
}
