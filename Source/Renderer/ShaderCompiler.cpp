#include "Renderer/ShaderCompiler.h"

#include "Renderer/ResourceSet.h"
#include "Renderer/ShaderIncluder.h"
#include "shaderc/env.h"
#include "shaderc/shaderc.h"
#include "shaderc/shaderc.hpp"
#include "shaderc/status.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Axiom {
    ShaderCompiler::ShaderCompiler(std::filesystem::path includeDir) : includeDir(includeDir), compiler() {
    }

    std::expected<ShaderSources, std::string> ShaderCompiler::split(const std::string& axsSource) {
        size_t vertexSrcStart = axsSource.find("#type vertex");
        size_t fragmentSrcStart = axsSource.find("#type fragment");

        if (vertexSrcStart == std::string::npos || fragmentSrcStart == std::string::npos) {
            return std::unexpected<std::string>("Shader does not contain valid '#type' tags");
        }

        return ShaderSources{.vertex = axsSource.substr(vertexSrcStart + 13, fragmentSrcStart - (vertexSrcStart + 13)),
                             .fragment = axsSource.substr(fragmentSrcStart + 15, std::string::npos)};
    }

    std::expected<CompiledShader, std::string> ShaderCompiler::compile(const ShaderSources& src, const std::string& name) const {
        shaderc::CompileOptions options = makeOptions();

        shaderc::CompilationResult vertCompResult =
            compiler.CompileGlslToSpv(src.vertex.c_str(), src.vertex.size(), shaderc_shader_kind::shaderc_vertex_shader, name.c_str(), "main", options);
        if (vertCompResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            return std::unexpected<std::string>(vertCompResult.GetErrorMessage());
        }

        shaderc::CompilationResult fragCompResult =
            compiler.CompileGlslToSpv(src.fragment.c_str(), src.fragment.size(), shaderc_shader_kind::shaderc_fragment_shader, name.c_str(), "main", options);
        if (vertCompResult.GetCompilationStatus() != shaderc_compilation_status_success) {
            return std::unexpected<std::string>(fragCompResult.GetErrorMessage());
        }

        return CompiledShader{.vertex = std::vector<uint32_t>(vertCompResult.begin(), vertCompResult.end()),
                              .fragment = std::vector<uint32_t>(fragCompResult.begin(), fragCompResult.end())};
    }

    shaderc::CompileOptions ShaderCompiler::makeOptions() const {
        shaderc::CompileOptions options{};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
        options.SetIncluder(std::make_unique<ShaderIncluder>(includeDir));
        options.AddMacroDefinition("SET_GLOBAL_DATA", std::to_string(Bindings::GlobalDataSet));
        options.AddMacroDefinition("BINDING_GLOBAL_DATA", std::to_string(Bindings::GlobalDataBinding));
        options.AddMacroDefinition("SET_BINDLESS_TEXTURE", std::to_string(Bindings::BindlessTextureSet));
        options.AddMacroDefinition("BINDING_BINDLESS_TEXTURE", std::to_string(Bindings::BindlessTextureBinding));
        options.AddMacroDefinition("BINDING_BINDLESS_TEXTURE_SAMPLERS", std::to_string(Bindings::BindlessTextureSamplersBinding));

        return options;
    }

} // namespace Axiom
