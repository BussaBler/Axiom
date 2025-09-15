#include "axpch.h"
#include "Instance.h"
#include "Platform/Vulkan/VulkanInstance.h"

namespace Axiom {
	std::unique_ptr<Instance> Instance::create(GraphicsAPI api) {
		Instance* instance = nullptr;
		switch (api) {
		case GraphicsAPI::VULKAN:
			instance = new VulkanInstance();
			currentInstance = instance;
			return std::unique_ptr<Instance>(instance);
		case GraphicsAPI::DIRECTX12:
			AX_CORE_LOG_ERROR("DirectX12 not supported yet!");
			return nullptr;
		default:
			AX_CORE_LOG_ERROR("Unknown Graphics API!");
			return nullptr;
		}
	}
}
