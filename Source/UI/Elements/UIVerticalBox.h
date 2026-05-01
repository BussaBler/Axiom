#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIVerticalBox : public UIElement {
      public:
        UIVerticalBox() = default;
        ~UIVerticalBox() = default;

        Math::Vec2 getDesiredSize() override;
        void arrange(const Math::Vec2& position, const Math::Vec2& size) override;
    };
} // namespace Axiom