#pragma once
#include "UIElement.h"

namespace Axiom {
    class UICanvas : public UIElement {
      public:
        UICanvas() = default;
        ~UICanvas() = default;

        void arrange(const Math::Vec2& position, const Math::Vec2& size) override;
    };
}