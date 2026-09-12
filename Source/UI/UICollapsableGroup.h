#pragma once

#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"

#include <string>

namespace Axiom {
    class UICollapsableGroup : public UIContainer {
      public:
        explicit UICollapsableGroup(const std::string& title) : title(title) {}
        ~UICollapsableGroup() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

      private:
        std::string title;
        bool isActive = false;
        bool isOpen = false;

        float cachedHeaderHeight = 0.0f;
    };
} // namespace Axiom
