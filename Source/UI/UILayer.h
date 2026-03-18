#pragma once
#include "Core/Layer.h"
#include "UI.h"

namespace Axiom {
	class UILayer : public Layer {
	public:
		UILayer() : Layer("UILayer") {}
		~UILayer() = default;

		void onAttach() override;
		void onDetach() override;
		void onEvent(Event& event) override;
		void onRender(CommandBuffer* commandBuffer) override;

		void begin();
		void end();
	};
}

