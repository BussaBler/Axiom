#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Axiom {
    struct FileInfo {
        std::string name;
        bool isDirectory;
        uint64_t size;
    };

    class FileSystem {
      public:
        static void mount(const std::string& virtualPrefix, const std::filesystem::path& physicalPath);
        static std::filesystem::path resolvePath(const std::filesystem::path& virtualPath);

        static bool exists(const std::filesystem::path& filePath);
        static std::vector<uint8_t> readFile(const std::filesystem::path& filePath);
        static std::string readFileStr(const std::filesystem::path& filePath);
        static void writeFile(const std::filesystem::path& filePath, std::vector<uint8_t>& data);
        static void writeFile(const std::filesystem::path& filePath, const std::string& data) {
            std::vector<uint8_t> bytes(data.begin(), data.end());
            writeFile(filePath, bytes);
        }
        static std::vector<FileInfo> getDirectory(const std::filesystem::path& folderPath);
        static void createDirectory(const std::filesystem::path& folderPath);
        static void setWorkingDirectory(const std::filesystem::path& folderPath);
        static const std::filesystem::path& getWorkingDirectory() { return workingDirectory; }

      private:
        static std::filesystem::path workingDirectory;
        static std::unordered_map<std::string, std::filesystem::path> mounts;

        inline static constexpr std::string ENGINE_VIRTUAL_PATH = "axiom://";
    };
} // namespace Axiom
