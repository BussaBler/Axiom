#pragma once

#include "UI/UIElement.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Axiom {
    class UIDropdown : public UIElement {
      public:
        UIDropdown() = default;
        ~UIDropdown() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        inline UIDropdown& setOptions(const std::vector<std::string>& newOptions) {
            options = newOptions;
            invalidateLayout();
            return *this;
        }
        inline UIDropdown& setValueGetter(std::function<int()> getter) {
            valueGetter = std::move(getter);
            return *this;
        }
        inline UIDropdown& setValueSetter(std::function<void(int)> setter) {
            valueSetter = std::move(setter);
            return *this;
        }

      private:
        std::vector<std::string> options;

        std::function<int()> valueGetter;
        std::function<void(int)> valueSetter;

        bool isOpen = false;
        bool isActive = false;
        int hoveredIndex = -1;
        int activePressedIndex = -1;
    };
} // namespace Axiom
