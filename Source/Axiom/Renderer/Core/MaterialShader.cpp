#include "axpch.h"
#include "MaterialShader.h"
#include "Platform/Vulkan/Shader/VulkanMaterialShader.h"

namespace Axiom {
	std::unique_ptr<MaterialShader> MaterialShader::create(Device& device) {
		return std::make_unique<VulkanMaterialShader>(static_cast<VulkanDevice&>(device));
	}
}
