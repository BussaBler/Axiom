#pragma once
#include "Renderer/Pipeline.h"
#include "Core/Assert.h"
#include "VulkanUtils.h"
#include "VulkanResourceLayout.h"

namespace Axiom {
	class VulkanPipeline : public Pipeline {
	public:
		VulkanPipeline(const CreateInfo& createInfo, Vk::Device logicDevice);
		~VulkanPipeline() override;

		inline Vk::Pipeline getHandle() const { return pipeline; }
		inline Vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }

	private:
		Vk::ShaderModule createShaderModule(std::filesystem::path shaderPath);
		Vk::PolygonMode axPolygonToVkPolygon(PolygonMode mode);
		Vk::CullModeFlags axCullModeToVkCullMode(CullMode mode);

	private:
		Vk::Device device = nullptr;
		Vk::Pipeline pipeline = nullptr;
		Vk::PipelineLayout pipelineLayout = nullptr;
	};
}

