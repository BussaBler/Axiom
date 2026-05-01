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
        Color panelBackgroundColor = Color::cyan();
        Color headerBackgroundColor = Color::darkGray();

        Color controlNormalColor = Color::lightGray();
        Color controlHoverColor = Color::gray();
        Color controlActiveColor = Color::darkGray();

        Color textColor = Color::white();
        float fontSize = 8.0f;

        UIMargin margin{5.0f, 5.0f, 5.0f, 5.0f};
        UIMargin padding{5.0f, 5.0f, 5.0f, 5.0f};
        Math::Vec4 borderRadius = Math::Vec4(5.0f);

        static std::shared_ptr<UITheme> getDefault() {
            static std::shared_ptr<UITheme> defaultTheme = std::make_shared<UITheme>();
            return defaultTheme;
        }
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
        inline UIElement* getParent() const { return parent; }
        inline const std::vector<std::shared_ptr<UIElement>>& getChildren() const { return children; }

        virtual Math::Vec2 getDesiredSize() { return desiredSize; }
        virtual void arrange(const Math::Vec2& position, const Math::Vec2& size) {
            arrangedPosition = position;
            arrangedSize = size;
            for (const auto& child : children) {
                child->arrange(position, size);
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

        virtual void onRender(UIRenderer* renderer) {
            for (const auto& child : children) {
                child->onRender(renderer);
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
        void setTheme(std::shared_ptr<UITheme> newTheme) { customTheme = newTheme; }
        void setHorizontalAlignment(UIAlignment alignment) { horizontalAlignment = alignment; }
        void setVerticalAlignment(UIAlignment alignment) { verticalAlignment = alignment; }
        void setFixedSize(const Math::Vec2& size) { fixedSize = size; }

        const std::string& getID() const { return id; }
        UIAlignment getHorizontalAlignment() const { return horizontalAlignment; }
        UIAlignment getVerticalAlignment() const { return verticalAlignment; }
        const Math::Vec2& getFixedSize() const { return fixedSize; }
        const std::shared_ptr<UITheme>& getTheme() const { return resolvedTheme; }

      protected:
        UIElement* parent = nullptr;
        std::vector<std::shared_ptr<UIElement>> children;

        std::string id;

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