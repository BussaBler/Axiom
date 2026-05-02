#include "UIImage.h"

namespace Axiom {
    Math::Vec2 UIImage::getDesiredSize() {
        Math::Vec2 size = Math::Vec2::zero();

        if (fixedSize.x() > 0) {
            size.x() = fixedSize.x();
        }
        if (fixedSize.y() > 0) {
            size.y() = fixedSize.y();
        }

        desiredSize.x() = size.x() + padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() = size.y() + padding.top + padding.bottom + margin.top + margin.bottom;
        return desiredSize;
    }

    void UIImage::onRender(UIRenderer* uiRenderer) {
        if (texture) {
            uiRenderer->addImageQuad(arrangedPosition, arrangedSize, texture.get());
        }
        UIElement::onRender(uiRenderer);
    }
} // namespace Axiom