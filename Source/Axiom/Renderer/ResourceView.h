#pragma once
#include "axpch.h"

namespace Axiom {
	class ResourceView {
	public:
		virtual ~ResourceView() = default;

		virtual uint32_t generateId() = 0;
	};
}