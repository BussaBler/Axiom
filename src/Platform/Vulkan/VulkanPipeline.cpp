#include "axpch.h"
#include "VulkanPipeline.h"

namespace Axiom {
	VulkanPipeline::VulkanPipeline(std::vector<std::filesystem::path> shaderStages) {
		
	}

	void VulkanPipeline::bind() {
		AX_CORE_LOG_TRACE("VulkanPipeline bind called");
	}

	void VulkanPipeline::unbind() {
		AX_CORE_LOG_TRACE("VulkanPipeline unbind called");
	}
}
