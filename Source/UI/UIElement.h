#pragma once

#include "Event/Event.h"
#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "Math/Color.h"
#include "Math/Vec.h"
#include "UI/UIEdgeInsets.h"
#include "UI/UIRenderer.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Axiom {
    struct UITheme {
        Color windowBackgroundColor = Color(0.06f, 0.06f, 0.07f);
        Color panelBackgroundColor = Color(0.10f, 0.10f, 0.12f);
        Color headerBackgroundColor = Color(0.14f, 0.13f, 0.16f);

        Color controlNormalColor = Color(0.16f, 0.15f, 0.19f);
        Color controlHoverColor = Color(0.24f, 0.22f, 0.28f);
        Color controlActiveColor = Color(0.32f, 0.28f, 0.38f);

        Color accentColor = Color(0.55f, 0.35f, 0.90f);
        Color accentHoverColor = Color(0.65f, 0.45f, 0.95f);

        Color goldAccent = Color(0.85f, 0.65f, 0.20f);
        Color goldHover = Color(0.95f, 0.75f, 0.30f);

        Color textPrimary = Color(0.90f, 0.90f, 0.92f);
        Color textMuted = Color(0.58f, 0.55f, 0.62f);

        UIEdgeInsets panelPadding = UIEdgeInsets(8.0f);
        UIEdgeInsets containerPadding = UIEdgeInsets(4.0f);
        UIEdgeInsets itemMargin = UIEdgeInsets(0.0f, 0.0f, 0.0f, 4.0f);

        float defaultRowHeight = 24.0f;
        float headerRowHeight = 28.0f;

        float fontSize = 14.0f;
        Math::Vec4 borderRadius = Math::Vec4(4.0f);

        static const UITheme& getDefault() {
            static const UITheme defaultTheme{};
            return defaultTheme;
        }
    };

    struct UIContext {
        UIRenderer* renderer;
        const UITheme* theme;
        uint8_t layer = 0;
    };

    class UICanvas;

    class UIElement {
      public:
        UIElement() = default;
        virtual ~UIElement() = default;

        virtual Math::Vec2 getDesiredSize(const UIContext& context) { return desiredSize; };
        virtual void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
            arrangedPosition = position;
            arrangedSize = size;
        }

        virtual void onRender(const UIContext& context, const Math::Rect& scissorRect) {}
        virtual bool onEvent(Event& event) {
            if (event.isHandled()) {
                return true;
            }

            EventDispatcher dispatcher(event);
            dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) {
                float mouseX = e.getMouseX();
                float mouseY = e.getMouseY();

                isHovered = (mouseX >= arrangedPosition.x() && mouseX <= arrangedPosition.x() + arrangedSize.x() && mouseY >= arrangedPosition.y() &&
                             mouseY <= arrangedPosition.y() + arrangedSize.y());

                return false;
            });

            return false;
        }

        virtual void invalidateLayout() {
            if (parent) {
                parent->invalidateLayout();
            }
        }
        virtual bool isCanvas() { return false; }

        UIElement& setParent(UIElement* newParent) {
            parent = newParent;
            return *this;
        }
        UIElement& setId(const std::string& newId) {
            id = newId;
            return *this;
        }

        UIElement* getParent() const { return parent; }
        UICanvas* getParentCanvas();
        const std::string& getId() const { return id; }
        Math::Vec2 getArrangedPosition() const { return arrangedPosition; }
        Math::Vec2 getArrangedSize() const { return arrangedSize; }
        Math::Vec2 getDesiredSize() const { return desiredSize; }

      protected:
        UIElement* parent = nullptr;
        std::string id;

        Math::Vec2 arrangedPosition = Math::Vec2::zero();
        Math::Vec2 arrangedSize = Math::Vec2::zero();
        Math::Vec2 desiredSize = Math::Vec2::zero();

        bool isHovered = false;
    };
} // namespace Axiom
