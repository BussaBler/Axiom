#pragma once
#include "Renderer/Shader.h"
#include "Core/Assert.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanShader : public Shader {
	public:
		VulkanShader(std::filesystem::path& filePath);

		std::vector<uint32_t> getShaderByteCode() override;
		std::string getShaderSourceCode() override;
	};
}

