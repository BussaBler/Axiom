
#include "UIPanel.h"

#include "Math/AxMath.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"

namespace Axiom {
    void UIPanel::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, overrideBackgroundColor.value_or(context.theme->panelBackgroundColor),
                                       context.theme->borderRadius, context.layer);
        UIContainer::onRender(context, scissorRect);
    }
} // namespace Axiom
