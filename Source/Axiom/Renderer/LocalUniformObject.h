#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class LocalUniformObject {
	public:
		LocalUniformObject() = default;
		LocalUniformObject(Math::Vec4 diffuseColor, Math::Vec4 r0, Math::Vec4 r1, Math::Vec4 r2);
		~LocalUniformObject() = default;

		void setDiffuseColor(const Math::Vec4& color) { diffuseColor = color; }

	private:
		Math::Vec4 diffuseColor;
		Math::Vec4 reserved0;
		Math::Vec4 reserved1;
		Math::Vec4 reserved2;
	};
}

