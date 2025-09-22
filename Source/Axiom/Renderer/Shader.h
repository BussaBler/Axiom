#pragma once

namespace Axiom {
	class CommandBuffer;

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void bind(CommandBuffer& commandBuffer) const = 0;
	};
}
