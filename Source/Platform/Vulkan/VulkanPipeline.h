#pragma once
#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Axiom {
	struct PipelineConfigInfo {

	};

	class VulkanPipeline : public Pipeline {
	public:
		VulkanPipeline(const std::string& name, std::vector<std::filesystem::path>& shaderStages, VulkanDevice& graphicsDevice);
		~VulkanPipeline() = default; 
		void bind() override;
		void unbind() override;

		static void defaultPipelineCreateInfo(uint32_t width, uint32_t height, PipelineConfigInfo& configInfo);

	private:
		VulkanDevice& device;
	};
}

