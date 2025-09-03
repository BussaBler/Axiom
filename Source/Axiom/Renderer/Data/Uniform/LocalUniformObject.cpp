#include "axpch.h"
#include "LocalUniformObject.h"

namespace Axiom {
	LocalUniformObject::LocalUniformObject(Math::Vec4 diffuseColor, Math::Vec4 r0, Math::Vec4 r1, Math::Vec4 r2) 
		: diffuseColor(diffuseColor), reserved0(r0), reserved1(r1), reserved2(r2) {}
}
