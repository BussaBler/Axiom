#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIPanel : public UIElement {
      public:
        UIPanel() = default;
        ~UIPanel() = default;

        Math::Vec2 getDesiredSize() override;
        void arrange(const Math::Vec2& position, const Math::Vec2& size) override;

        void onRender(UIRenderer* uiRenderer) override;

        void setBackgroundColor(const Color& color) { overridePanelBackgroundColor = color; }

      private:
        std::optional<Color> overridePanelBackgroundColor;
    };
} // namespace Axiom