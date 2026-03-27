#include "axpch.h"
#include "Layer.h"
#include "Application.h"

namespace Axiom {
	void Layer::requestLayerAction(std::unique_ptr<Layer> newLayer, ActionType action) {
		Application::get()->queueLayerAction(this, std::move(newLayer), action);
	}
}
