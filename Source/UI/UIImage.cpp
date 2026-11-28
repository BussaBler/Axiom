#include "axpch.h"

#include "UIImage.h"

namespace Axiom {
    void UIImage::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        if (!imageTexture) {
            return;
        }

        Math::Vec2 position = arrangedPosition;
        Math::Vec2 size = arrangedSize;

        const Math::iVec2 textureSize = imageTexture->getSize();
        if (preserveAspect && textureSize.x() > 0 && textureSize.y() > 0 && size.x() > 0.0f && size.y() > 0.0f) {
            const float textureAspect = static_cast<float>(textureSize.x()) / static_cast<float>(textureSize.y());
            const float areaAspect = size.x() / size.y();
            const Math::Vec2 fitted = areaAspect > textureAspect ? Math::Vec2(size.y() * textureAspect, size.y())
                                                                 : Math::Vec2(size.x(), size.x() / textureAspect);
            position += (size - fitted) / 2.0f;
            size = fitted;
        }

        context.renderer->addImageQuad(position, size, imageTexture.get(), context.layer);
    }
} // namespace Axiom
