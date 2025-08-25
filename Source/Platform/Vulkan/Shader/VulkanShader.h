#pragma once
#include "Core/Assert.h"
#include "Utils/FileSystem.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>

namespace Axiom {
	struct VulkanShadersStage {
		VkShaderModule handle;
		VkShaderModuleCreateInfo createInfo;
		VkPipelineShaderStageCreateInfo stageCreateInfo;
	};

	class VulkanShader {
	public:
		VulkanShader(VulkanDevice& vkDevice);
		virtual ~VulkanShader();
		virtual void use(VulkanCommandBuffer& commandBuffer) = 0;
		virtual void createPipeline(VulkanRenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) = 0;

	protected:
		void createShaderModule(const std::string& shaderName, const std::string& shaderStageStr, VkShaderStageFlagBits shaderStageFlag, uint32_t index);
		std::vector<uint32_t> getShaderByteCode(std::filesystem::path filePath, VkShaderStageFlagBits shaderStageFlag);

	protected:
		VulkanDevice& device;
		std::unique_ptr<VulkanPipeline> pipeline;
		std::vector<VulkanShadersStage> shaderStages;

	private:
		std::filesystem::path shaderDir;
	};
}

