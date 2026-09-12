#include "axpch.h"

#include "UIImage.h"

namespace Axiom {
    void UIImage::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        if (imageTexture) {
            context.renderer->addImageQuad(arrangedPosition, arrangedSize, imageTexture.get(), context.layer);
        }
    }
} // namespace Axiom
