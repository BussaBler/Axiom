#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIDropdown : public UIElement {
      public:
        UIDropdown(const std::vector<std::string>& options) : options(options) {}
        ~UIDropdown() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        void setSelectedIndex(int index) {
            if (index >= 0 && index < static_cast<int>(options.size())) {
                selectedIndex = index;
            }
        }
        int getSelectedIndex() const { return selectedIndex; }

        void setOnSelectionChanged(const std::function<void(int, std::string)>& callback) { onSelectionChanged = std::move(callback); }

      private:
        std::vector<std::string> options;
        int selectedIndex = -1;
        bool isActive = false;
        bool isOpen = false;

        int hoveredIndex = -1;
        int activeIndex = -1;

        std::function<void(int, std::string)> onSelectionChanged;
    };
} // namespace Axiom