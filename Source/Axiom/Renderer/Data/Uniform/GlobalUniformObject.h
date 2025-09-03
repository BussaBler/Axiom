#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class GlobalUniformObject {
	public:
		GlobalUniformObject() = default;
		GlobalUniformObject(Math::Mat4 projection, Math::Mat4 view, Math::Mat4 r0, Math::Mat4 r1);
		~GlobalUniformObject() = default;

		void setProjection(const Math::Mat4& proj) { projectionMatrix = proj; }
		void setView(const Math::Mat4& v) { viewMatrix = v; }

	private:
		Math::Mat4 projectionMatrix;
		Math::Mat4 viewMatrix;
		Math::Mat4 reserved0;
		Math::Mat4 reserved1;
	};
}

