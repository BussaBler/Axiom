#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIDragFloat : public UIElement {
      public:
        Math::Vec2 getDesiredSize() override;

        void onRender(UIRenderer* uiRenderer) override;
        bool onEvent(Event& event) override;

        void setDragSpeed(float speed) { dragSpeed = speed; }
        void setNormalColor(const Color& color) { overrideNormalColor = color; }
        void setHoverColor(const Color& color) { overrideHoverColor = color; }
        void setActiveColor(const Color& color) { overrideActiveColor = color; }
        void setFloatLimit(const Math::Vec2& limit) {
            floatLimit = limit;
            isLimitted = true;
        }
        void unsetFloatLimit() { isLimitted = false; }
        void setValueGetter(std::function<float()> getter) { getValue = std::move(getter); }
        void setValueSetter(std::function<void(float)> setter) { setValue = std::move(setter); }

      private:
        float dragSpeed = 0.1f;
        std::optional<Color> overrideNormalColor;
        std::optional<Color> overrideHoverColor;
        std::optional<Color> overrideActiveColor;

        bool isHovered = false;
        bool isActive = false;

        bool isLimitted = false;
        Math::Vec2 floatLimit = Math::Vec2::zero();

        float lastMouseX = 0.0f;

        std::function<void(float)> setValue = nullptr;
        std::function<float()> getValue = nullptr;
    };
} // namespace Axiom