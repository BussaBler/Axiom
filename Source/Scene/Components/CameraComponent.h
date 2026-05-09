#pragma once
#include "Core/Base.h"
#include "Renderer/Camera.h"

namespace Axiom {
    AX_COMPONENT() struct CameraComponent {
        Camera camera;
        bool isMainCamera = false;
        bool isFixedAspectRatio = false;
    };
} // namespace Axiom