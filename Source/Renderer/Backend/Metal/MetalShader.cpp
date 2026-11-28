#include "axpch.h"

#include "MetalShader.h"

#include "Renderer/Backend/Metal/MetalBindingLayout.h"
#include "Renderer/Backend/Metal/MetalResourceLayout.h"
#include "spirv_cross_error_handling.hpp"

#include <SpirvCross/spirv_msl.hpp>
#include <cstdint>
#include <expected>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <span>
#include <string>

namespace Axiom {
    MetalShader::MetalShader(std::span<const uint32_t> vertexSpirv, std::span<const uint32_t> fragmentSpirv)
        : vertexSpirv(vertexSpirv.begin(), vertexSpirv.end()), fragmentSpirv(fragmentSpirv.begin(), fragmentSpirv.end()) {
    }

    std::expected<std::string, std::string> MetalShader::translateSPIRVtoMSL(std::span<const uint32_t> spirv,
                                                                             std::span<const ResourceLayout* const> setLayouts) {
        try {
            spirv_cross::CompilerMSL mslCompiler(spirv.data(), spirv.size());
            spirv_cross::CompilerMSL::Options mslOptions = {};
            mslOptions.platform = spirv_cross::CompilerMSL::Options::Platform::macOS;
            mslOptions.set_msl_version(3, 0);
            mslOptions.argument_buffers = true;
            mslOptions.argument_buffers_tier = spirv_cross::CompilerMSL::Options::ArgumentBuffersTier::Tier2;
            mslOptions.ios_support_base_vertex_instance = true;
            mslCompiler.set_msl_options(mslOptions);

            spv::ExecutionModel executionModel = mslCompiler.get_execution_model();

            spirv_cross::MSLResourceBinding pushConstantMap = {};
            pushConstantMap.stage = executionModel;
            pushConstantMap.desc_set = spirv_cross::kPushConstDescSet;
            pushConstantMap.binding = spirv_cross::kPushConstBinding;
            pushConstantMap.msl_buffer = AX_METAL_PUSH_CONST_BUFFER_INDEX;
            mslCompiler.add_msl_resource_binding(pushConstantMap);

            spirv_cross::ShaderResources resources = mslCompiler.get_shader_resources();
            std::vector<uint32_t> mappedSets;

            auto mapDescriptorSets = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources) {
                for (const auto& resource : resources) {
                    uint32_t set = mslCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                    uint32_t binding = mslCompiler.get_decoration(resource.id, spv::DecorationBinding);
                    if (std::find(mappedSets.begin(), mappedSets.end(), set) == mappedSets.end()) {
                        spirv_cross::MSLResourceBinding argBinding = {};
                        argBinding.stage = executionModel;
                        argBinding.desc_set = set;

                        argBinding.binding = spirv_cross::kArgumentBufferBinding;
                        argBinding.msl_buffer = AX_METAL_DESC_SET_START_INDEX + set;

                        mslCompiler.add_msl_resource_binding(argBinding);
                        mslCompiler.set_argument_buffer_device_address_space(set, true);
                        mappedSets.push_back(set);
                    }

                    spirv_cross::MSLResourceBinding resourceBinding = {};
                    resourceBinding.stage = executionModel;
                    resourceBinding.desc_set = set;
                    resourceBinding.binding = binding;
                    const auto* layout = static_cast<const MetalResourceLayout*>(setLayouts[set]);
                    const uint32_t index = layout->getFirstIndex(binding);
                    resourceBinding.msl_buffer = index;
                    resourceBinding.msl_texture = index;
                    resourceBinding.msl_sampler = index;
                    resourceBinding.count = layout->getCount(binding);

                    mslCompiler.add_msl_resource_binding(resourceBinding);
                }
            };

            mapDescriptorSets(resources.uniform_buffers);
            mapDescriptorSets(resources.storage_buffers);
            mapDescriptorSets(resources.sampled_images);
            mapDescriptorSets(resources.separate_images);
            mapDescriptorSets(resources.separate_samplers);

            return mslCompiler.compile();
        } catch (const spirv_cross::CompilerError& error) {
            return std::unexpected(error.what());
        }
    }
} // namespace Axiom
