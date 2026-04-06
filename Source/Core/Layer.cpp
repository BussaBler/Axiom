#include "Layer.h"
#include "Application.h"
#include "axpch.h"

namespace Axiom {
    void Layer::requestLayerAction(std::unique_ptr<Layer> newLayer, ActionType action) {
        Application::get()->queueLayerAction(this, std::move(newLayer), action);
    }
} // namespace Axiom
