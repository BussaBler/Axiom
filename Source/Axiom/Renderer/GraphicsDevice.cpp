#include "axpch.h"
#include "GraphicsDevice.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Axiom {
	std::unique_ptr<GraphicsDevice> GraphicsDevice::create(Window* window, GraphicsAPI api) {
		switch (api) {
			case GraphicsAPI::None:
				AX_CORE_LOG_ERROR("GraphicsAPI::None is not supported!");
				return nullptr;
			case GraphicsAPI::Vulkan:
				return std::make_unique<VulkanDevice>(window);
			case GraphicsAPI::DirectX:
				// return new DirectXGraphicsDevice();
			default:
				AX_CORE_LOG_ERROR("Unknown GraphicsAPI!");
				return nullptr;
		}
	}
}
