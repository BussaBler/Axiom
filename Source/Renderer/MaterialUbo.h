#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	struct MaterialUbo {
		Math::Vec4 diffuseColor{ 1.0f,1.0f,1.0f,1.0f };
		Math::Vec4 pad0{ 1.0f,1.0f,1.0f,1.0f };
		Math::Vec4 pad1{ 1.0f,1.0f,1.0f,1.0f };
		Math::Vec4 pad2{ 1.0f,1.0f,1.0f,1.0f };
	};
}
