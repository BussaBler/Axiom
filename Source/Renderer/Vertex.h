#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class Vertex {
	public:
		Vertex(Math::Vec4 position, Math::Vec4 normal, Math::Vec4 uvwz);
		~Vertex() = default;

	public:
		Math::Vec4 position = Math::Vec4(0.0f);
		Math::Vec4 normal = Math::Vec4(0.0f);
		Math::Vec4 uv = Math::Vec4(0.0f);
	};
}

