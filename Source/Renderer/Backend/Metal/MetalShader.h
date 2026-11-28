#pragma once
#include "MetalUtils.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/Shader.h"

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace Axiom {
    class MetalShader : public Shader {
      public:
        MetalShader(std::span<const uint32_t> vertexSpirv, std::span<const uint32_t> fragmentSpirv);
        ~MetalShader() = default;

        static std::expected<std::string, std::string> translateSPIRVtoMSL(std::span<const uint32_t> spirv, std::span<const ResourceLayout* const> setLayouts);

        const std::vector<uint32_t>& getVertexSpirv() const { return vertexSpirv; }
        const std::vector<uint32_t>& getFragmentSpirv() const { return fragmentSpirv; }

      private:
      private:
        std::vector<uint32_t> vertexSpirv;
        std::vector<uint32_t> fragmentSpirv;
    };
} // namespace Axiom
