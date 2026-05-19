#pragma once
#include "Event/MouseEvent.h"
#include "UIContainer.h"

namespace Axiom {
    class UIScrollBox : public UIContainer {
      public:
        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

      private:
        Math::Vec2 scrollOffset{0.0f, 0.0f};
    };
} // namespace Axiom