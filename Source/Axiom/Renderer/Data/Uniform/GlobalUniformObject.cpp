#include "axpch.h"
#include "GlobalUniformObject.h"

namespace Axiom {
	GlobalUniformObject::GlobalUniformObject(Math::Mat4 projection, Math::Mat4 view, Math::Mat4 r0, Math::Mat4 r1)
		: projectionMatrix(projection), viewMatrix(view), reserved0(r0), reserved1(r1) {}
}
