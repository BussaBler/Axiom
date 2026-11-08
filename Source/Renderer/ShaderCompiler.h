#pragma once

#include "shaderc/shaderc.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace Axiom {
    struct ShaderSources {
        std::string vertex;
        std::string fragment;
    };

    struct CompiledShader {
        std::vector<uint32_t> vertex;
        std::vector<uint32_t> fragment;
    };

    class ShaderCompiler {
      public:
        explicit ShaderCompiler(std::filesystem::path includeDir);

        static std::expected<ShaderSources, std::string> split(const std::string& axsSource);
        std::expected<CompiledShader, std::string> compile(const ShaderSources& src, const std::string& name) const;

      private:
        shaderc::CompileOptions makeOptions() const;

      private:
        std::filesystem::path includeDir;
        shaderc::Compiler compiler;
    };
} // namespace Axiom
