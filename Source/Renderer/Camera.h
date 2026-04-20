#pragma once
#include "Math/AxMath.h"

namespace Axiom {
    class Camera {
      public:
        virtual ~Camera() = default;

        inline void setOrtographic(float left, float right, float bottom, float top, float near, float far) {
            projection = Math::Mat4::orthographic(left, right, bottom, top, near, far);
        }
        inline void setPerspective(float fov, float aspectRatio, float near, float far) { projection = Math::Mat4::perspective(fov, aspectRatio, near, far); }
        void setProjection(const Math::Mat4& newProj) { projection = newProj; }

        const Math::Mat4& getProjection() const { return projection; }

      protected:
        Math::Mat4 projection = Math::Mat4::identity();
    };
} // namespace Axiom