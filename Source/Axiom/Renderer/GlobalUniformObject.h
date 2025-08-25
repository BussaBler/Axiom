#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class GlobalUniformObject {
	public:
		GlobalUniformObject() = default;
		~GlobalUniformObject() = default;

	public:
		Math::Mat4 projection;
		Math::Mat4 view;
		Math::Mat4 reserved0;
		Math::Mat4 reserved1;
	};
}

