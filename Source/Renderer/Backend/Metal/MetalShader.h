#pragma once
#include "MetalUtils.h"
#include "Renderer/Shader.h"

namespace Axiom {
    class MetalShader : public Shader {
      public:
        MetalShader(const std::string& vertexSource, const std::string& fragmentSource, MTL::Device* device);
        ~MetalShader();

        MTL::Library* getLibrary() const { return library; }

      private:
        std::vector<uint32_t> compileGLSLToSPIRV(const std::string& source);
        std::string compileSPIRVtoMSL(const std::vector<uint32_t>& spirv);

      private:
        MTL::Library* library = nullptr;
    };
} // namespace Axiom