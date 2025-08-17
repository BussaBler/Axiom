#include "axpch.h"
#include "RendererContext.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Axiom {
	std::unique_ptr<RendererContext> RendererContext::create() {
		return std::make_unique<VulkanContext>();
	}
}