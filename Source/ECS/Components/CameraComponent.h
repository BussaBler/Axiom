#pragma once
#include "Core/Base.h"
#include "Renderer/Camera.h"

namespace Axiom {
    struct AX_COMPONENT() CameraComponent {
        Camera camera;
        AX_PROPERTY() bool isMainCamera = false;
        AX_PROPERTY() bool isFixedAspectRatio = false;
    };
} // namespace Axiom