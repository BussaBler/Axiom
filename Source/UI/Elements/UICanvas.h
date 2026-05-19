#pragma once
#include "UIContainer.h"

namespace Axiom {
    class UICanvas : public UIContainer {
      public:
        UICanvas() = default;
        ~UICanvas() = default;

        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
    };
}