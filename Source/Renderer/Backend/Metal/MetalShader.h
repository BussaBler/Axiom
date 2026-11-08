#pragma once
#include "MetalUtils.h"
#include "Renderer/Shader.h"

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace Axiom {
    class MetalShader : public Shader {
      public:
        MetalShader(std::span<const uint32_t> vertexSpirv, std::span<const uint32_t> fragmentSpirv, MTL::Device* device);
        ~MetalShader();

        inline MTL::Library* getVertexLibrary() const { return vertexLibrary; }
        inline MTL::Library* getFragmentLibrary() const { return fragmentLibrary; }

      private:
        std::expected<std::string, std::string> compileSPIRVtoMSL(std::span<const uint32_t> spirv);

      private:
        MTL::Library* vertexLibrary = nullptr;
        MTL::Library* fragmentLibrary = nullptr;
    };
} // namespace Axiom
