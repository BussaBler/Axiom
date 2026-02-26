#include "axpch.h"
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "Renderer/Vertex.h"

namespace Axiom {
	VulkanPipeline::~VulkanPipeline() {
		device.waitIdle();
		if (pipeline) {
			device.getHandle().destroyPipeline(pipeline);
		}
		if (pipelineLayout) {
			device.getHandle().destroyPipelineLayout(pipelineLayout);
		}
	}

	void VulkanPipeline::init(VulkanPipelineCreateInfo& createInfo) {
		Vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, createInfo.descriptorSetLayouts, createInfo.pushConstantRanges);
		Vk::ResultValue<Vk::PipelineLayout> layoutResult = device.getHandle().createPipelineLayout(pipelineLayoutInfo);

		if (layoutResult.result != Vk::Result::eSuccess) {
			AX_CORE_LOG_ERROR("Failed to create Vulkan Pipeline Layout!");
			return;
		}
		pipelineLayout = layoutResult.value;

		// Viewport State
		Vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);
		// Vertex Input State
		Vk::VertexInputBindingDescription bindingDescription(0, sizeof(Vertex), Vk::VertexInputRate::eVertex);
		Vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescription, createInfo.vertexAttributes);
		// Input Assembly
		Vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, Vk::PrimitiveTopology::eTriangleList, Vk::False);
		// Rasterizer
		Vk::PolygonMode polygonMode = createInfo.isWireframe ? Vk::PolygonMode::eLine : Vk::PolygonMode::eFill;
		Vk::PipelineRasterizationStateCreateInfo rasterizer({}, Vk::False, Vk::False, polygonMode, Vk::CullModeFlagBits::eBack, Vk::FrontFace::eCounterClockwise, Vk::False);
		rasterizer.setLineWidth(1.0f);
		// Multisampling
		Vk::PipelineMultisampleStateCreateInfo multisampling({}, Vk::SampleCountFlagBits::e1, Vk::False);
		// Color Blending
		Vk::ColorComponentFlags colorWriteMask = Vk::ColorComponentFlagBits::eR | Vk::ColorComponentFlagBits::eG | Vk::ColorComponentFlagBits::eB | Vk::ColorComponentFlagBits::eA;
		Vk::PipelineColorBlendAttachmentState colorBlendAttachment(Vk::True, Vk::BlendFactor::eSrcAlpha, Vk::BlendFactor::eOneMinusSrcAlpha, Vk::BlendOp::eAdd, Vk::BlendFactor::eSrcAlpha, Vk::BlendFactor::eOneMinusSrcAlpha, Vk::BlendOp::eAdd, colorWriteMask);
		Vk::PipelineColorBlendStateCreateInfo colorBlending({}, Vk::False, Vk::LogicOp::eClear, colorBlendAttachment);
		// Depth and Stencil
		Vk::PipelineDepthStencilStateCreateInfo depthStencil({}, Vk::True, Vk::True, Vk::CompareOp::eLess, Vk::False, Vk::False);
		// Dynamic State
		std::array<Vk::DynamicState, 3> dynamicStates = {
			Vk::DynamicState::eViewport,
			Vk::DynamicState::eScissor,
			Vk::DynamicState::eLineWidth
		};
		Vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

		Vk::GraphicsPipelineCreateInfo pipelineInfo(
			{},
			createInfo.shaderStages,
			&vertexInputInfo,
			&inputAssembly,
			{},
			&viewportState,
			&rasterizer,
			&multisampling,
			&depthStencil,
			&colorBlending,
			&dynamicState,
			pipelineLayout,
			createInfo.renderPass
		);
		Vk::ResultValue<Vk::Pipeline> pipelineResult = device.getHandle().createGraphicsPipeline(nullptr, pipelineInfo);

		AX_CORE_ASSERT(pipelineResult.result == Vk::Result::eSuccess, "Failed to create Vulkan Graphics Pipeline!");
		pipeline = pipelineResult.value;
	}

	void VulkanPipeline::bind(Vk::CommandBuffer commandBuffer, Vk::PipelineBindPoint bindPoint) const {
		commandBuffer.bindPipeline(bindPoint, pipeline);
	}
}
