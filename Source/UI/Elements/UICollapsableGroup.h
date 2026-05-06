#pragma once
#include "UIElement.h"

namespace Axiom {
    class UICollapsableGroup : public UIElement {
      public:
        explicit UICollapsableGroup(const std::string& title) : title(title) {}
        ~UICollapsableGroup() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        void onRender(const UIContext& context) override;
        bool onEvent(Event& event) override;

      private:
        static constexpr float HEADER_HEIGHT = 30.0f;
        static constexpr float HEADER_TEXT_PADDING_X = 5.0f;
        static constexpr float HEADER_TEXT_PADDING_Y = 4.0f;

        std::string title;
        bool isOpen = false;
        bool isHovered = false;
        float headerHeight = HEADER_HEIGHT;
    };
} // namespace Axiom