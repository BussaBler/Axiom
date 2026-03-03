#pragma once
#include "Renderer/Pipeline.h"
#include "Core/Assert.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanPipeline : public Pipeline {
	public:
		VulkanPipeline(const CreateInfo& createInfo, Vk::Device logicDevice);
		~VulkanPipeline() override;

	private:
		Vk::ShaderModule createShaderModule(std::filesystem::path shaderPath);
		Vk::PolygonMode AxPolygonToVkPolygon(PolygonMode mode);
		Vk::CullModeFlags AxCullModeToVkCullMode(CullMode mode);

	private:
		Vk::Device device = nullptr;
		Vk::Pipeline pipeline = nullptr;
		Vk::PipelineLayout pipelineLayout = nullptr;
	};
}

