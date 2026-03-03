#include "VulkanPipeline.h"
#include "Utils/FileSystem.h"

namespace Axiom {
	VulkanPipeline::VulkanPipeline(const CreateInfo& createInfo, Vk::Device logicDevice) : device(logicDevice) {
		Vk::ShaderModule vertShaderModule = createShaderModule(createInfo.vertexShaderPath);
		Vk::ShaderModule fragShaderModule = createShaderModule(createInfo.fragmentShaderPath);

		Vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, Vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
		Vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, Vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
		std::array<Vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		Vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, {}, {});

		Vk::PrimitiveTopology vkTopology = (createInfo.topology == Axiom::PrimitiveTopology::LineList) ? Vk::PrimitiveTopology::eLineList : Vk::PrimitiveTopology::eTriangleList;
		Vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vkTopology, Vk::False);
		
		Vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);
		std::array<Vk::DynamicState, 2> dynamicStates = { Vk::DynamicState::eViewport, Vk::DynamicState::eScissor };
		Vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

		Vk::FrontFace vkFrontFace = createInfo.frontFaceClockwise ? Vk::FrontFace::eClockwise : Vk::FrontFace::eCounterClockwise;
		Vk::PipelineRasterizationStateCreateInfo rasterizer({}, Vk::False, Vk::False, AxPolygonToVkPolygon(createInfo.polygonMode), AxCullModeToVkCullMode(createInfo.cullMode), vkFrontFace);
		rasterizer.setLineWidth(1.0f);

		Vk::PipelineMultisampleStateCreateInfo multisampling({}, Vk::SampleCountFlagBits::e1);

		Vk::PipelineDepthStencilStateCreateInfo depthStencil({},
			createInfo.enableDepthTest ? Vk::True : Vk::False,
			createInfo.enableDepthWrite ? Vk::True : Vk::False,
			Vk::CompareOp::eLess, Vk::False, Vk::False
		);

		Vk::PipelineColorBlendAttachmentState colorBlendAttachment(createInfo.enableBlending ? Vk::True : Vk::False);
		colorBlendAttachment.setColorWriteMask(Vk::ColorComponentFlagBits::eR | Vk::ColorComponentFlagBits::eG | Vk::ColorComponentFlagBits::eB | Vk::ColorComponentFlagBits::eA);
		if (createInfo.enableBlending) {
			colorBlendAttachment.setSrcColorBlendFactor(Vk::BlendFactor::eSrcAlpha);
			colorBlendAttachment.setDstColorBlendFactor(Vk::BlendFactor::eOneMinusSrcAlpha);
			colorBlendAttachment.setColorBlendOp(Vk::BlendOp::eAdd);
			colorBlendAttachment.setSrcAlphaBlendFactor(Vk::BlendFactor::eOne);
			colorBlendAttachment.setDstAlphaBlendFactor(Vk::BlendFactor::eZero);
			colorBlendAttachment.setAlphaBlendOp(Vk::BlendOp::eAdd);
		}

		Vk::PipelineColorBlendStateCreateInfo colorBlending({}, Vk::False, Vk::LogicOp::eCopy, 1, &colorBlendAttachment);
		colorBlending.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

		Vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, {});
		Vk::ResultValue<Vk::PipelineLayout> pipelineLayoutResult = device.createPipelineLayout(pipelineLayoutInfo);

		AX_CORE_ASSERT(pipelineLayoutResult.result == Vk::Result::eSuccess, "Failed to create pipeline layout!");
		pipelineLayout = pipelineLayoutResult.value;

		std::vector<Vk::Format> vkColorFormats;
		for (auto fmt : createInfo.colorAttachmentFormats) {
			vkColorFormats.push_back(AxFormatToVkFormat(fmt));
		}
		Vk::Format vkDepthFormat = AxFormatToVkFormat(createInfo.depthAttachmentFormat);

		Vk::PipelineRenderingCreateInfo renderingInfo({}, vkColorFormats, vkDepthFormat, Vk::Format::eUndefined);
	
		Vk::GraphicsPipelineCreateInfo pipelineInfo(
			{}, shaderStages, &vertexInputInfo, &inputAssembly, nullptr,
			&viewportState, &rasterizer, &multisampling, &depthStencil,
			&colorBlending, &dynamicState, pipelineLayout
		);
		pipelineInfo.setPNext(&renderingInfo);
		Vk::ResultValue<Vk::Pipeline> pipelineResult = device.createGraphicsPipeline(nullptr, pipelineInfo);

		AX_CORE_ASSERT(pipelineResult.result == Vk::Result::eSuccess, "Failed to create graphics pipeline!");
		pipeline = pipelineResult.value;

		device.destroyShaderModule(vertShaderModule);
		device.destroyShaderModule(fragShaderModule);
	}

	VulkanPipeline::~VulkanPipeline() {
		if (pipelineLayout) {
			device.destroyPipelineLayout(pipelineLayout);
		}
		if (pipeline) {
			device.destroyPipeline(pipeline);
		}
	}

	Vk::ShaderModule VulkanPipeline::createShaderModule(std::filesystem::path shaderPath) {
		std::vector<uint8_t> shaderCode = FileSystem::readFile(shaderPath);

		Vk::ShaderModuleCreateInfo shaderModuleCreateInfo({}, static_cast<uint32_t>(shaderCode.size()), reinterpret_cast<const uint32_t*>(shaderCode.data()));
		Vk::ResultValue<Vk::ShaderModule> shaderModuleResult = device.createShaderModule(shaderModuleCreateInfo);

		AX_CORE_ASSERT(shaderModuleResult.result == Vk::Result::eSuccess, "Failed to create shader module!");
		return shaderModuleResult.value;
	}

	Vk::PolygonMode VulkanPipeline::AxPolygonToVkPolygon(PolygonMode mode) {
		switch (mode) {
			case Axiom::PolygonMode::Fill: return Vk::PolygonMode::eFill;
			case Axiom::PolygonMode::Line: return Vk::PolygonMode::eLine;
			case Axiom::PolygonMode::Point: return Vk::PolygonMode::ePoint;
			default: return Vk::PolygonMode::eFill;
		}
	}

	Vk::CullModeFlags VulkanPipeline::AxCullModeToVkCullMode(CullMode mode) {
		switch (mode) {
			case Axiom::CullMode::None: return Vk::CullModeFlagBits::eNone;
			case Axiom::CullMode::Front: return Vk::CullModeFlagBits::eFront;
			case Axiom::CullMode::Back: return Vk::CullModeFlagBits::eBack;
			default: return Vk::CullModeFlagBits::eBack;
		}
	}
}
