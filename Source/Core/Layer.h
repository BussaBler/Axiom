#pragma once
#include "Event/Event.h"
#include "Renderer/CommandBuffer.h"

namespace Axiom {
	class Layer {
	public:
		Layer(const char* name = "Layer") : debugName(name) {}
		virtual ~Layer() = default;
		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(Event& event) {}
		virtual void onUIRender() {}
		virtual void onRender(CommandBuffer* commandBuffer) {}
		const std::string& getName() const { return debugName; }

	protected:
		std::string debugName;
	};
}

