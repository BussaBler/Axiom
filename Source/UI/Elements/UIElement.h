#pragma once
#include "Event/Event.h"
#include "Math/AxMath.h"
#include "UI/UIRenderer.h"
#include "axpch.h"

namespace Axiom {
    enum class UIAlignment { Fill, Start, Center, End };

    struct UIMargin {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
    };

    struct UITheme {
        Color panelBackgroundColor = Color(0.11f, 0.11f, 0.11f, 1.0f);
        Color headerBackgroundColor = Color(0.06f, 0.06f, 0.06f, 1.0f);

        Color controlNormalColor = Color(0.18f, 0.18f, 0.18f, 1.0f);
        Color controlHoverColor = Color(0.25f, 0.25f, 0.25f, 1.0f);
        Color controlActiveColor = Color(0.00f, 0.45f, 0.85f, 1.0f);

        Color textColor = Color(0.85f, 0.85f, 0.85f, 1.0f);
        float fontSize = 8.0f;

        Math::Vec4 borderRadius = Math::Vec4(6.0f);

        static std::shared_ptr<UITheme> getDefault() {
            static std::shared_ptr<UITheme> defaultTheme = std::make_shared<UITheme>();
            return defaultTheme;
        }
    };

    struct UIContext {
        UIRenderer* renderer;
        float dpiScale;
        uint8_t layer = 0;
    };

    class UIElement {
      public:
        UIElement() = default;
        virtual ~UIElement() = default;

        inline void addChild(std::shared_ptr<UIElement> child) {
            child->parent = this;
            children.push_back(child);
            child->resolveTheme();
        }
        inline void removeChild(std::shared_ptr<UIElement> child) {
            child->parent = nullptr;
            children.erase(std::remove(children.begin(), children.end(), child), children.end());
        }
        inline void clearChildren() {
            for (auto& child : children) {
                child->parent = nullptr;
            }
            children.clear();
        }
        inline UIElement* getParent() const { return parent; }
        inline const std::vector<std::shared_ptr<UIElement>>& getChildren() const { return children; }

        virtual Math::Vec2 getDesiredSize(const UIContext& context) { return desiredSize; }
        virtual void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
            arrangedPosition = position;
            arrangedSize = size;
            for (const auto& child : children) {
                child->arrange(context, position, size);
            }
        }
        void resolveTheme() {
            if (customTheme) {
                resolvedTheme = customTheme;
            } else if (parent) {
                resolvedTheme = parent->resolvedTheme;
            }

            for (const auto& child : children) {
                child->resolveTheme();
            }
        }

        virtual void onRender(const UIContext& context) {
            for (const auto& child : children) {
                child->onRender(context);
            }
        }
        virtual bool onEvent(Event& event) {
            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                if ((*it)->onEvent(event)) {
                    return true;
                }
            }
            return false;
        }

        void setID(const std::string& newID) { id = newID; }
        void setMargin(const UIMargin& newMargin) { margin = newMargin; }
        void setPadding(const UIMargin& newPadding) { padding = newPadding; }
        void setTheme(std::shared_ptr<UITheme> newTheme) { customTheme = newTheme; }
        void setHorizontalAlignment(UIAlignment alignment) { horizontalAlignment = alignment; }
        void setVerticalAlignment(UIAlignment alignment) { verticalAlignment = alignment; }
        void setFixedSize(const Math::Vec2& size) { fixedSize = size; }

        const std::string& getID() const { return id; }
        const UIMargin& getMargin() const { return margin; }
        const UIMargin& getPadding() const { return padding; }
        UIAlignment getHorizontalAlignment() const { return horizontalAlignment; }
        UIAlignment getVerticalAlignment() const { return verticalAlignment; }
        Math::Vec2 getArrangedPosition() const { return arrangedPosition; }
        Math::Vec2 getArrangedSize() const { return arrangedSize; }
        const Math::Vec2& getFixedSize() const { return fixedSize; }
        const std::shared_ptr<UITheme>& getTheme() const { return resolvedTheme; }

      protected:
        UIElement* parent = nullptr;
        std::vector<std::shared_ptr<UIElement>> children;

        std::string id;

        UIMargin margin;
        UIMargin padding;

        UIAlignment horizontalAlignment = UIAlignment::Fill;
        UIAlignment verticalAlignment = UIAlignment::Fill;

        Math::Vec2 arrangedPosition;
        Math::Vec2 arrangedSize;

        Math::Vec2 desiredSize = Math::Vec2::zero();
        Math::Vec2 fixedSize = Math::Vec2(-1.0f);

        std::shared_ptr<UITheme> resolvedTheme = UITheme::getDefault();

      private:
        std::shared_ptr<UITheme> customTheme = nullptr;
    };

} // namespace Axiom