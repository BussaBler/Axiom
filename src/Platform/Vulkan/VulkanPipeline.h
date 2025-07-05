#pragma once
#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

namespace Axiom {
	class VulkanPipeline : public Pipeline {
	public:
		VulkanPipeline(std::vector<std::filesystem::path> shaderStages);
		~VulkanPipeline() override = default; 
		void bind() override;
		void unbind() override;
	};
}

