#include "axpch.h"
#include "LayerStack.h"

namespace Axiom {
	LayerStack::LayerStack() {}

	LayerStack::~LayerStack() {
		// The management of the layer memory is left to the palce where it whas pushed to the stack, so we don't delete the layers here
		//for (Layer* layer : layers) {
		//	layer->onDetach();
		//	delete layer;
		//}
	}

	void LayerStack::pushLayer(Layer* layer) {
		layers.emplace(layers.begin() + layerInsertIndex, layer);
		layerInsertIndex++;
		layer->onAttach();
	}

	void LayerStack::pushOverlay(Layer* overlay) {
		layers.emplace_back(overlay);
		overlay->onAttach();
	}

	void LayerStack::popLayer(Layer* layer) {
		auto it = std::find(layers.begin(), layers.end(), layer);
		if (it != layers.end()) {
			layer->onDetach();
			layers.erase(it);
			layerInsertIndex--;
		}
	}

	void LayerStack::popOverlay(Layer* overlay) {
		auto it = std::find(layers.begin(), layers.end(), overlay);
		if (it != layers.end()) {
			overlay->onDetach();
			layers.erase(it);
		}
	}
}
