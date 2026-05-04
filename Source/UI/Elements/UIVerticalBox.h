#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIVerticalBox : public UIElement {
      public:
        UIVerticalBox() = default;
        ~UIVerticalBox() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
    };
} // namespace Axiom