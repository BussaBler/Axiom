#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIImage : public UIElement {
      public:
        UIImage() = default;
        ~UIImage() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context) override;

        void setTexture(const std::shared_ptr<Texture>& newTexture) { texture = newTexture; }

      private:
        std::shared_ptr<Texture> texture = nullptr;
    };
} // namespace Axiom