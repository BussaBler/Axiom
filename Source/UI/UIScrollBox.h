#pragma once

#include "Event/MouseEvent.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"

namespace Axiom {
    class UIScrollBox : public UIContainer {
      public:
        UIScrollBox() = default;
        ~UIScrollBox() = default;

        Math::Vec2 measure(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        bool onScroll(const MouseScrolledEvent& event) override;

      private:
        Math::Vec2 scrollOffset = Math::Vec2::zero();
    };
} // namespace Axiom
