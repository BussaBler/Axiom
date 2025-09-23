#pragma once
#include "Renderer/Shader.h"
#include "VulkanPipeline.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanPipeline;
	class VulkanBuffer;

	struct VulkanShaderStage {
		VkShaderModuleCreateInfo createInfo{};
		VkShaderModule module = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo stageInfo{};
	};

	class VulkanShader : public Shader {
	public:
		VulkanShader(VulkanDevice& vkDevice, std::string fileName) : device(vkDevice), shaderName(fileName) {}
		~VulkanShader() override;

		void init(VkRenderPass vkRenderPass);
		void bind(CommandBuffer& commandBuffer) const override;

	protected:
		void createShaderModules();
		virtual void createPipeline(VkRenderPass vkRenderPass) {};
		virtual void createDescriptors() {};

	protected:
		VulkanDevice& device;
		constexpr static size_t MAX_SHADER_STAGES = 2; // Vertex and Fragment
		std::array<VulkanShaderStage, MAX_SHADER_STAGES> shaderStages{};
		static std::filesystem::path shadeDir;
		std::string shaderName;
		std::unique_ptr<VulkanPipeline> pipeline = nullptr;
	};
}

