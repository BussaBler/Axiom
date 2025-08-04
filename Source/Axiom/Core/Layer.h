#pragma once
#include "Event/Event.h"

namespace Axiom {
	class Layer {
	public:
		Layer(const char* name = "Layer") : debugName(name) {}
		virtual ~Layer() = default;
		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate() {}
		virtual void onEvent(Event& event) {}
		const std::string& getName() const { return debugName; }

	protected:
		std::string debugName;
	};
}

