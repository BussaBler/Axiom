#pragma once

namespace Axiom {
	class Pipeline {
		public:
		Pipeline() = default;
		virtual ~Pipeline() = default;
		virtual void bind() = 0; //TODO: add a command buffer parameter
		virtual void unbind() = 0;
	};
}
