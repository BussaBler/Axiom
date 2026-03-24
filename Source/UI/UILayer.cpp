#include "UILayer.h"
#include "Event/MouseEvent.h"

namespace Axiom {
	void UILayer::onAttach() {
		UI::init();
	}

	void UILayer::onDetach() {
		UI::shutdown();
	}

	void UILayer::onUpdate() {
		UI::beginFrame();
	}

	void UILayer::onEvent(Event& event) {
		EventDispatcher dispatcher(event);

		dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) {
			UI::setMousePosition({ e.getMouseX(), e.getMouseY() });
			return UI::shouldConsumeMouseEvents();
		});

		dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) {
			UI::setMouseButtonState(e.getMouseButton(), true);
			return UI::shouldConsumeMouseEvents();
		});
		dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) {
			UI::setMouseButtonState(e.getMouseButton(), false);
			return UI::shouldConsumeMouseEvents();
		});
	}

	void UILayer::onRender(CommandBuffer* commandBuffer) {
		UI::endFrame();
		UI::render(commandBuffer);
	}
}