#pragma once
#include "Core/Base.h"
#include "Math/Mat.h"
#include "Math/Vec.h"

namespace Axiom {
    struct AX_COMPONENT() TransformComponent {
        AX_PROPERTY() Math::Vec3 position = Math::Vec3::zero();
        AX_PROPERTY() Math::Vec3 rotation = Math::Vec3::zero();
        AX_PROPERTY() Math::Vec3 scale = Math::Vec3::one();

        void translate(const Math::Vec3& translation) { position += translation; }
        void rotate(const Math::Vec3& eulerOffset) { rotation += eulerOffset; }
        void multiplyScale(const Math::Vec3& scaleModifier) { scale *= scaleModifier; }
        void multiplyScale(float uniformScale) { scale *= uniformScale; }

        Math::Mat4 modelMatrix() const { return Math::Mat4::model(position, rotation, scale); }
        Math::Vec3 getForward() const {
            Math::Mat4 rotMatrix = Math::Mat4::model(Math::Vec3::zero(), rotation, Math::Vec3::zero());
            return Math::normalize(Math::Vec3(rotMatrix[0][2], rotMatrix[1][2], rotMatrix[2][2]));
        }
        Math::Vec3 getRight() const {
            Math::Mat4 rotMatrix = Math::Mat4::model(Math::Vec3::zero(), rotation, Math::Vec3::one());
            return Math::normalize(Math::Vec3(rotMatrix[0][0], rotMatrix[1][0], rotMatrix[2][0]));
        }
        Math::Vec3 getUp() const {
            Math::Mat4 rotMatrix = Math::Mat4::model(Math::Vec3::zero(), rotation, Math::Vec3::one());
            return Math::normalize(Math::Vec3(rotMatrix[0][1], rotMatrix[1][1], rotMatrix[2][1]));
        }
    };
} // namespace Axiom
