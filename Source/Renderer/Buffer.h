#pragma once

namespace Axiom {
	class Buffer {
	public:

		size_t getSize();
		virtual void* map() = 0;
		virtual void unmap() = 0;

	};
}