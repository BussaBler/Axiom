#pragma once

#include "Math/Vec.h"
#include "UI/UIElement.h"

#include <functional>

namespace Axiom {
    class UICheckbox : public UIElement {
      public:
        UICheckbox() = default;
        ~UICheckbox() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        inline UICheckbox& setNormalColor(Color color) {
            overrideNormalColor = color;
            return *this;
        }
        inline UICheckbox& setHoverColor(Color color) {
            overrideHoverColor = color;
            return *this;
        }
        inline UICheckbox& setActiveColor(Color color) {
            overrideActiveColor = color;
            return *this;
        }

        inline UICheckbox& setValueGetter(std::function<bool()> getter) {
            valueGetter = std::move(getter);
            return *this;
        }
        inline UICheckbox& setValueSetter(std::function<void(bool)> setter) {
            valueSetter = std::move(setter);
            return *this;
        }

      private:
        std::optional<Color> overrideNormalColor;
        std::optional<Color> overrideHoverColor;
        std::optional<Color> overrideActiveColor;

        bool isActive = false;

        std::function<bool()> valueGetter;
        std::function<void(bool)> valueSetter;
    };
} // namespace Axiom
