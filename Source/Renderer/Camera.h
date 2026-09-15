#pragma once
#include "Math/Mat.h"
#include "Math/Vec.h"

namespace Axiom {
    class Camera {
      public:
        virtual ~Camera() = default;

        inline void setOrtographic(float left, float right, float bottom, float top, float near, float far) {
            projection = Math::Mat4::orthographic(left, right, bottom, top, near, far);
        }
        inline void setPerspective(float fov, float aspectRatio, float near, float far) { projection = Math::Mat4::perspective(fov, aspectRatio, near, far); }
        inline void setProjection(const Math::Mat4& newProj) { projection = newProj; }
        inline void setView(const Math::Mat4& newView) { view = newView; };
        inline void setPosition(const Math::Vec3& newPosition) { position = newPosition; }

        inline const Math::Mat4& getProjection() const { return projection; }
        inline const Math::Mat4& getView() const { return view; }
        inline const Math::Vec3& getPosition() const { return position; }

      protected:
        Math::Mat4 projection = Math::Mat4::identity();
        Math::Mat4 view = Math::Mat4::identity();
        Math::Vec3 position = Math::Vec3::zero();
    };
} // namespace Axiom
