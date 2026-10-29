#pragma once

#include "shaderc/shaderc.hpp"

#include <string>
namespace Axiom {
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
      public:
        shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type, const char* requestingSource, size_t includeDepth) override;
        void ReleaseInclude(shaderc_include_result* data) override;

      private:
        struct IncludeData {
            std::string sourceName;
            std::string content;
        };
    };
} // namespace Axiom
