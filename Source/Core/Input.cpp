#include "Input.h"

namespace Axiom {
    std::array<bool, 512> Input::keyStates = {};
    std::array<bool, 32> Input::mouseButtonStates = {};
    Math::Vec2 Input::mousePosition = Math::Vec2(0.0f, 0.0f);
}