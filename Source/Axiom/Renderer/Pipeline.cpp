#include "axpch.h"
#include "Pipeline.h"

namespace Axiom {
	std::unique_ptr<Pipeline> Pipeline::create(const std::string& name, std::vector<const std::filesystem::path>& shaderStages, GraphicsDevice& graphicsDevice) {
		switch (graphicsDevice.getAPI())
		{
		case GraphicsAPI::Vulkan:
				// return std::make_unique<VulkanPipeline>(name, shaderStages, graphicsDevice);
			break;
		case GraphicsAPI::DirectX:
				// return std::make_unique<DirectXPipeline>(name, shaderStages, graphicsDevice);
			break;
		default:
			return nullptr;
		}
	}
}