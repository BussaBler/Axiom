#include "axpch.h"
#include "Pipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Axiom {
	std::unique_ptr<Pipeline> Pipeline::create(Device& deviceRef, RenderPass& renderPassRef, const PipelineConfigInfo& configInfo) {
		return std::make_unique<VulkanPipeline>(static_cast<VulkanDevice&>(deviceRef), renderPassRef, configInfo);
	}
}
