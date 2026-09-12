#pragma once

#include "Math/Color.h"
#include "UI/UIElement.h"

#include <optional>
#include <string>

namespace Axiom {
    class UIText : public UIElement {
      public:
        UIText(const std::string& text) : text(text) {}
        ~UIText() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        UIText& setText(const std::string& newText) {
            text = newText;
            return *this;
        }
        UIText& setTextColor(const Color& color) {
            textColor = color;
            return *this;
        }

      private:
        std::string text;
        Math::Vec2 textSize = Math::Vec2::zero();
        std::optional<Color> textColor;
    };
} // namespace Axiom
