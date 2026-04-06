#pragma once
namespace Axiom {
    class StringUtils {
      public:
        static std::string trim(const std::string &str);
        static std::vector<std::string> split(const std::string &str, char delimiter);
    };
} // namespace Axiom
