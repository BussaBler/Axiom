#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIPanel : public UIElement {
      public:
        UIPanel() = default;
        ~UIPanel() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        void setBackgroundColor(const Color& color) { overridePanelBackgroundColor = color; }

      private:
        std::optional<Color> overridePanelBackgroundColor;
    };
} // namespace Axiom