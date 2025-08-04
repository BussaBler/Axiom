#include "axpch.h"
#include "VulkanPipeline.h"

namespace Axiom {
	VulkanPipeline::VulkanPipeline(const std::string& name, std::vector<std::filesystem::path>& shaderStages, VulkanDevice& graphicsDevice) 
		: Pipeline(name), device{ graphicsDevice } {
	}

	void VulkanPipeline::bind() {
		AX_CORE_LOG_TRACE("VulkanPipeline bind called");
	}

	void VulkanPipeline::unbind() {
		AX_CORE_LOG_TRACE("VulkanPipeline unbind called");
	}
}
