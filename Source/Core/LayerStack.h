#pragma once
#include "Layer.h"

namespace Axiom {
    class LayerStack {
      public:
        LayerStack() = default;
        ~LayerStack() {
            for (auto& layer : layers) {
                layer->onDetach();
            }
        }

        template <typename T>
            requires std::derived_from<T, Layer>
        void pushLayer() {
            layers.emplace(layers.begin() + layerInsertIndex, std::make_unique<T>());
            layerInsertIndex++;
            layers[layerInsertIndex - 1]->onAttach();
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void pushOverlay() {
            layers.emplace_back(std::make_unique<T>());
            layers.back()->onAttach();
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void popLayer() {
            auto it = std::find_if(layers.begin(), layers.end(), [](const std::unique_ptr<Layer>& layer) { return isLayerOfType<T>(layer.get()); });
            it->onDetach();
            layers.erase(it);
            layerInsertIndex--;
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void popOverlay() {
            auto it = std::find_if(layers.begin(), layers.end(), [](const std::unique_ptr<Layer>& layer) { return isLayerOfType<T>(layer.get()); });
            it->onDetach();
            layers.erase(it);
        }

        void insertLayer(std::vector<std::unique_ptr<Layer>>::iterator it, std::unique_ptr<Layer> layer) {
            layers.insert(it, std::move(layer));
            layerInsertIndex++;
        }
        void eraseLayer(std::vector<std::unique_ptr<Layer>>::iterator it) {
            layers.erase(it);
            layerInsertIndex--;
        }

        std::vector<std::unique_ptr<Layer>>::iterator begin() {
            return layers.begin();
        }
        std::vector<std::unique_ptr<Layer>>::iterator end() {
            return layers.end();
        }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rbegin() {
            return layers.rbegin();
        }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rend() {
            return layers.rend();
        }

        std::vector<std::unique_ptr<Layer>>::const_iterator begin() const {
            return layers.begin();
        }
        std::vector<std::unique_ptr<Layer>>::const_iterator end() const {
            return layers.end();
        }
        std::vector<std::unique_ptr<Layer>>::const_reverse_iterator rbegin() const {
            return layers.rbegin();
        }
        std::vector<std::unique_ptr<Layer>>::const_reverse_iterator rend() const {
            return layers.rend();
        }

      private:
        template <typename T> static bool isLayerOfType(Layer* layer) {
            if (auto type = dynamic_cast<T*>(layer)) {
                return true;
            }
            return false;
        }

      private:
        std::vector<std::unique_ptr<Layer>> layers;
        size_t layerInsertIndex = 0;
    };
} // namespace Axiom
