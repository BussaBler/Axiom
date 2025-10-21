#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	struct GlobalUbo {
		Math::Mat4 view;
		Math::Mat4 proj;
		Math::Vec4 pad0;
		Math::Vec4 pad1;
		Math::Vec4 pad2;
		Math::Vec4 pad3;
		//Math::Mat4 pad4;
	};
}
