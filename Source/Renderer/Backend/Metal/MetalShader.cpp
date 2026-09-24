#include "axpch.h"

#include "MetalShader.h"

#include "Core/Log.h"
#include "Renderer/Backend/Metal/MetalBindingLayout.h"
#include "spirv_cross_error_handling.hpp"

#include <SpirvCross/spirv_msl.hpp>
#include <cstdint>
#include <expected>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>

namespace Axiom {
    MetalShader::MetalShader(const std::string& vertexSource, const std::string& fragmentSource, MTL::Device* device) {
        std::expected<std::vector<uint32_t>, std::string> vertexSPIRV = compileGLSLToSPIRV(vertexSource);
        if (!vertexSPIRV.has_value()) {
            AX_CORE_LOG_ERROR("GLSL to SPIR-V Compilation Failed [Vertex]:\n{}", vertexSPIRV.error());
            return;
        }
        std::expected<std::vector<uint32_t>, std::string> fragmentSPIRV = compileGLSLToSPIRV(fragmentSource);
        if (!fragmentSPIRV.has_value()) {
            AX_CORE_LOG_ERROR("GLSL to SPIR-V Compilation Failed [Fragment]:\n{}", fragmentSPIRV.error());
            return;
        }

        std::expected<std::string, std::string> vertexMSL = compileSPIRVtoMSL(vertexSPIRV.value());
        if (!vertexMSL.has_value()) {
            AX_CORE_LOG_ERROR("SPIR-V to MSL Translation Failed [Vertex]:\n{}", vertexMSL.error());
            return;
        }
        std::expected<std::string, std::string> fragmentMSL = compileSPIRVtoMSL(fragmentSPIRV.value());
        if (!fragmentMSL.has_value()) {
            AX_CORE_LOG_ERROR("SPIR-V to MSL Translation Failed [Fragment]:\n{}", fragmentMSL.error());
            return;
        }

        NS::String* vertexMSLStr = NS::String::alloc()->init(vertexMSL.value().c_str(), NS::StringEncoding::UTF8StringEncoding);
        NS::String* fragmentMSLStr = NS::String::alloc()->init(fragmentMSL.value().c_str(), NS::StringEncoding::UTF8StringEncoding);
        MTL::CompileOptions* compileOptions = MTL::CompileOptions::alloc()->init();
        NS::Error* error = nullptr;

        vertexLibrary = device->newLibrary(vertexMSLStr, compileOptions, &error);
        if (error != nullptr) {
            AX_CORE_LOG_ERROR("Metal Backend Compilation Failed [Vertex]:\n{}", error->localizedDescription()->utf8String());
            compileOptions->release();
            vertexMSLStr->release();
            fragmentMSLStr->release();
            return;
        }

        fragmentLibrary = device->newLibrary(fragmentMSLStr, compileOptions, &error);
        if (error != nullptr) {
            AX_CORE_LOG_ERROR("Metal Backend Compilation Failed [Fragment]:\n{}", error->localizedDescription()->utf8String());
            compileOptions->release();
            vertexMSLStr->release();
            fragmentMSLStr->release();
            return;
        }

        compileOptions->release();
        vertexMSLStr->release();
        fragmentMSLStr->release();

        m_isCompiled = true;
    }

    MetalShader::~MetalShader() {
        if (vertexLibrary) {
            vertexLibrary->release();
            vertexLibrary = nullptr;
        }
        if (fragmentLibrary) {
            fragmentLibrary->release();
            fragmentLibrary = nullptr;
        }
    }

    std::expected<std::vector<uint32_t>, std::string> MetalShader::compileGLSLToSPIRV(const std::string& source) {
        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
        shaderc::CompilationResult compResult =
            compiler.CompileGlslToSpv(source.c_str(), source.size(), shaderc_shader_kind::shaderc_glsl_infer_from_source, "shader", "main", options);
        if (compResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            return std::unexpected(compResult.GetErrorMessage());
        }
        return std::vector<uint32_t>(compResult.begin(), compResult.end());
    }

    std::expected<std::string, std::string> MetalShader::compileSPIRVtoMSL(const std::vector<uint32_t>& spirv) {
        try {
            spirv_cross::CompilerMSL mslCompiler(spirv);
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
                    resourceBinding.msl_buffer = binding;
                    resourceBinding.msl_texture = binding;
                    resourceBinding.msl_sampler = binding;

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
