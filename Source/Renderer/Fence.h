#pragma once

namespace Axiom {
	class Fence {
	public:
		Fence() = default;
		virtual ~Fence() = default;

		virtual void wait() = 0;
		virtual void reset() = 0;
	};
}
