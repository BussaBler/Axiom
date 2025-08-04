#pragma once
#include "Core/Log.h"
#include "Core/Assert.h"
#include "Core/Window.h"

namespace Axiom {
	enum class GraphicsAPI {
		None = 0,
		Vulkan = 1,
		DirectX = 2
	};

	class GraphicsDevice {
	public:
		GraphicsDevice(Window* window, GraphicsAPI api) : window(window), api(api) {}
		virtual ~GraphicsDevice() = default;
		static std::unique_ptr<GraphicsDevice> create(Window* window, GraphicsAPI api);
		GraphicsAPI getAPI() const { return api; }

		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		virtual void submitCommandBuffer() = 0;

	protected:
		Window* window;

	private:
		GraphicsAPI api;
	};
}

