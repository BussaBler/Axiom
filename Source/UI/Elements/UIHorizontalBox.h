#pragma once
#include "UIContainer.h"

namespace Axiom {
    class UIHorizontalBox : public UIContainer {
      public:
        UIHorizontalBox() = default;
        ~UIHorizontalBox() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
    };
}