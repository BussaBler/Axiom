#pragma once

namespace Axiom {
    class Shader {
      public:
        Shader() = default;
        virtual ~Shader() = default;

        inline bool isCompiled() const { return m_isCompiled; }

      protected:
        bool m_isCompiled = false;
    };
} // namespace Axiom
