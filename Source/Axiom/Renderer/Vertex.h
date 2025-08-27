#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class Vertex {
	public:
		Vertex() = default;
		~Vertex() = default;

	public:
		Math::Vec3 position;
		Math::Vec2 texCoord;
	};
}

