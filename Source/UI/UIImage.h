#pragma once

#include "Renderer/Texture.h"
#include "UI/UIElement.h"
#include <memory>

namespace Axiom {
    class UIImage : public UIElement {
      public:
        UIImage() = default;
        ~UIImage() = default;

        void onRender(const UIContext &context, const Math::Rect &scissorRect) override;

        UIImage& setTexture(std::shared_ptr<Texture> newImageTex) {
            imageTexture = newImageTex;
            return *this;
        }

      private:
          std::shared_ptr<Texture> imageTexture = nullptr;
    };
}
