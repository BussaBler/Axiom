#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIHorizontalBox : public UIElement {
      public:
        UIHorizontalBox() = default;
        ~UIHorizontalBox() = default;

        Math::Vec2 getDesiredSize() override;
        void arrange(const Math::Vec2& position, const Math::Vec2& size) override;
    };
}