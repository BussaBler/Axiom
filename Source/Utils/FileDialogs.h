#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace Axiom {
    class FileDialogs {
      public:
        static std::optional<std::filesystem::path> openFolder(const std::string& title);
        static std::optional<std::filesystem::path> openFile(const std::string& title, const std::string& filter);
    };
} // namespace Axiom
