#pragma once
#include "Core/Base.h"

#include <string>

namespace Axiom {
    struct AX_COMPONENT() TagComponent {
        AX_PROPERTY() std::string tag;
    };
} // namespace Axiom