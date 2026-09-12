#pragma once

#include "Math/Color.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace Axiom {
    class UIButton : public UIElement {
      public:
        UIButton() = default;
        ~UIButton() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        UIButton& setText(const std::string& newText) {
            text = newText;
            return *this;
        }
        UIButton& setNormalColor(Color color) {
            overrideNormalColor = color;
            return *this;
        }
        UIButton& setHoverColor(Color color) {
            overrideHoverColor = color;
            return *this;
        }
        UIButton& setActiveColor(Color color) {
            overrideActiveColor = color;
            return *this;
        }
        UIButton& setFontSize(float size) {
            overrideFontSize = size;
            return *this;
        }
        UIButton& setOnClick(std::function<void()> callback) {
            onClickCallback = std::move(callback);
            return *this;
        }

      private:
        std::string text;
        Math::Vec2 textSize = Math::Vec2::zero();

        std::optional<Color> overrideNormalColor;
        std::optional<Color> overrideHoverColor;
        std::optional<Color> overrideActiveColor;
        std::optional<float> overrideFontSize;

        bool isActive = false;

        std::function<void()> onClickCallback;
    };
} // namespace Axiom
