#include "VulkanPipeline.h"
#include "Utils/FileSystem.h"
#include "VulkanResourceSet.h"
#include "VulkanShader.h"

namespace Axiom {
    VulkanPipeline::VulkanPipeline(const CreateInfo& createInfo, Vk::Device logicDevice, Vk::DescriptorPool descriptorPool)
        : device(logicDevice), descriptorPool(descriptorPool) {

        VulkanShader* vkShader = static_cast<VulkanShader*>(createInfo.shader);
        Vk::PipelineShaderStageCreateInfo vertShaderStageInfo({}, Vk::ShaderStageFlagBits::eVertex, vkShader->getVertexShaderModule(), "main");
        Vk::PipelineShaderStageCreateInfo fragShaderStageInfo({}, Vk::ShaderStageFlagBits::eFragment, vkShader->getFragmentShaderModule(), "main");
        std::array<Vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

        std::vector<Vk::VertexInputBindingDescription> bindingDescriptions(createInfo.vertexBindings.size());
        for (size_t i = 0; i < createInfo.vertexBindings.size(); i++) {
            const auto& binding = createInfo.vertexBindings[i];
            bindingDescriptions[i] = Vk::VertexInputBindingDescription(binding.binding, binding.stride,
                                                                       (binding.inputRate == VertexInputRate::Vertex) ? Vk::VertexInputRate::eVertex
                                                                                                                      : Vk::VertexInputRate::eInstance);
        }
        std::vector<Vk::VertexInputAttributeDescription> attributeDescriptions(createInfo.vertexAttributes.size());
        for (size_t i = 0; i < createInfo.vertexAttributes.size(); i++) {
            const auto& attribute = createInfo.vertexAttributes[i];
            attributeDescriptions[i] =
                Vk::VertexInputAttributeDescription(attribute.location, attribute.binding, axToVkFormat(attribute.format), attribute.offset);
        }
        Vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescriptions, attributeDescriptions);

        Vk::PrimitiveTopology vkTopology =
            (createInfo.topology == Axiom::PrimitiveTopology::LineList) ? Vk::PrimitiveTopology::eLineList : Vk::PrimitiveTopology::eTriangleList;
        Vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vkTopology, Vk::False);

        Vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);
        std::array<Vk::DynamicState, 2> dynamicStates = {Vk::DynamicState::eViewport, Vk::DynamicState::eScissor};
        Vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

        Vk::FrontFace vkFrontFace = createInfo.frontFaceClockwise ? Vk::FrontFace::eClockwise : Vk::FrontFace::eCounterClockwise;
        Vk::PipelineRasterizationStateCreateInfo rasterizer({}, Vk::False, Vk::False, axPolygonToVkPolygon(createInfo.polygonMode),
                                                            axCullModeToVkCullMode(createInfo.cullMode), vkFrontFace);
        rasterizer.setLineWidth(1.0f);

        Vk::PipelineMultisampleStateCreateInfo multisampling({}, Vk::SampleCountFlagBits::e1);

        Vk::PipelineDepthStencilStateCreateInfo depthStencil({}, createInfo.enableDepthTest ? Vk::True : Vk::False,
                                                             createInfo.enableDepthWrite ? Vk::True : Vk::False, Vk::CompareOp::eLess, Vk::False, Vk::False);

        Vk::PipelineColorBlendAttachmentState colorBlendAttachment(createInfo.enableBlending ? Vk::True : Vk::False);
        colorBlendAttachment.setColorWriteMask(Vk::ColorComponentFlagBits::eR | Vk::ColorComponentFlagBits::eG | Vk::ColorComponentFlagBits::eB |
                                               Vk::ColorComponentFlagBits::eA);
        if (createInfo.enableBlending) {
            colorBlendAttachment.setSrcColorBlendFactor(Vk::BlendFactor::eSrcAlpha);
            colorBlendAttachment.setDstColorBlendFactor(Vk::BlendFactor::eOneMinusSrcAlpha);
            colorBlendAttachment.setColorBlendOp(Vk::BlendOp::eAdd);
            colorBlendAttachment.setSrcAlphaBlendFactor(Vk::BlendFactor::eOne);
            colorBlendAttachment.setDstAlphaBlendFactor(Vk::BlendFactor::eZero);
            colorBlendAttachment.setAlphaBlendOp(Vk::BlendOp::eAdd);
        }

        Vk::PipelineColorBlendStateCreateInfo colorBlending({}, Vk::False, Vk::LogicOp::eCopy, 1, &colorBlendAttachment);
        colorBlending.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

        std::vector<Vk::DescriptorSetLayout> vkDescriptorSetLayouts;
        for (auto layout : createInfo.resourceLayouts) {
            auto vulkanLayout = static_cast<VulkanResourceLayout*>(layout);
            vkDescriptorSetLayouts.push_back(vulkanLayout->getHandle());
        }
        std::array<Vk::PushConstantRange, 1> pushConstantRanges = {
            Vk::PushConstantRange(Vk::ShaderStageFlagBits::eVertex | Vk::ShaderStageFlagBits::eFragment, 0, sizeof(Math::Mat4))};

        Vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, vkDescriptorSetLayouts, pushConstantRanges);
        Vk::ResultValue<Vk::PipelineLayout> pipelineLayoutResult = device.createPipelineLayout(pipelineLayoutInfo);

        AX_CORE_ASSERT(pipelineLayoutResult.result == Vk::Result::eSuccess, "Failed to create pipeline layout!");
        pipelineLayout = pipelineLayoutResult.value;

        std::vector<Vk::Format> vkColorFormats;
        for (auto fmt : createInfo.colorAttachmentFormats) {
            vkColorFormats.push_back(axToVkFormat(fmt));
        }
        Vk::Format vkDepthFormat = axToVkFormat(createInfo.depthAttachmentFormat);

        Vk::PipelineRenderingCreateInfo renderingInfo({}, vkColorFormats, vkDepthFormat, Vk::Format::eUndefined);

        Vk::GraphicsPipelineCreateInfo pipelineInfo({}, shaderStages, &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling,
                                                    &depthStencil, &colorBlending, &dynamicState, pipelineLayout);
        pipelineInfo.setPNext(&renderingInfo);
        Vk::ResultValue<Vk::Pipeline> pipelineResult = device.createGraphicsPipeline(nullptr, pipelineInfo);

        AX_CORE_ASSERT(pipelineResult.result == Vk::Result::eSuccess, "Failed to create graphics pipeline!");
        pipeline = pipelineResult.value;
    }

    VulkanPipeline::~VulkanPipeline() {
        if (pipelineLayout) {
            device.destroyPipelineLayout(pipelineLayout);
        }
        if (pipeline) {
            device.destroyPipeline(pipeline);
        }
    }

    std::unique_ptr<ResourceSet> VulkanPipeline::createResourceSet(ResourceLayout* resourceLayout) {
        return std::make_unique<VulkanResourceSet>(device, descriptorPool, static_cast<VulkanResourceLayout*>(resourceLayout)->getHandle());
    }

    Vk::PolygonMode VulkanPipeline::axPolygonToVkPolygon(PolygonMode mode) {
        switch (mode) {
        case Axiom::PolygonMode::Fill:
            return Vk::PolygonMode::eFill;
        case Axiom::PolygonMode::Line:
            return Vk::PolygonMode::eLine;
        case Axiom::PolygonMode::Point:
            return Vk::PolygonMode::ePoint;
        default:
            return Vk::PolygonMode::eFill;
        }
    }

    Vk::CullModeFlags VulkanPipeline::axCullModeToVkCullMode(CullMode mode) {
        switch (mode) {
        case Axiom::CullMode::None:
            return Vk::CullModeFlagBits::eNone;
        case Axiom::CullMode::Front:
            return Vk::CullModeFlagBits::eFront;
        case Axiom::CullMode::Back:
            return Vk::CullModeFlagBits::eBack;
        default:
            return Vk::CullModeFlagBits::eBack;
        }
    }
} // namespace Axiom
