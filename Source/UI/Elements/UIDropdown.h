#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIDropdown : public UIElement {
      public:
        UIDropdown(const std::vector<std::string>& options) : options(options) {}
        ~UIDropdown() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        void setSelectedIndex(int index) {
            if (index >= 0 && index < static_cast<int>(options.size())) {
                selectedIndex = index;
            }
        }
        int getSelectedIndex() const { return selectedIndex; }

      private:
        std::vector<std::string> options;
        int selectedIndex = -1;
        bool isOpen = false;
    };
} // namespace Axiom