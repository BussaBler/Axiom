#pragma once

#include "Math/Color.h"
#include "UI/UIContainer.h"

#include <optional>

namespace Axiom {
    class UIPanel : public UIContainer {
      public:
        UIPanel() = default;
        ~UIPanel() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        inline UIPanel& setBackgroundColor(const Color& color) {
            overrideBackgroundColor = color;
            return *this;
        }

      private:
        std::optional<Color> overrideBackgroundColor;
    };
} // namespace Axiom
