#include "axpch.h"

#include "FileSystem.h"

#include "Core/Log.h"

namespace Axiom {
    std::filesystem::path FileSystem::workingDirectory = std::filesystem::current_path();
    std::unordered_map<std::string, std::filesystem::path> FileSystem::mounts;

    void FileSystem::mount(const std::string& virtualPrefix, const std::filesystem::path& physicalPath) {
        mounts[virtualPrefix] = physicalPath;
        AX_CORE_LOG_INFO("Mounted VFS: {} to {}", virtualPrefix, physicalPath.generic_string());
    }

    std::filesystem::path FileSystem::resolvePath(const std::filesystem::path& virtualPath) {
        std::string vPathString = virtualPath.generic_string();
        size_t schemePos = vPathString.find("://");

        if (schemePos != std::string::npos) {
            std::string prefix = vPathString.substr(0, schemePos + 3);
            auto it = mounts.find(prefix);
            if (it != mounts.end()) {
                std::string sufix = vPathString.substr(prefix.length());
                return it->second / sufix;
            }
        }

        return virtualPath;
    }

    bool FileSystem::exists(const std::filesystem::path& filePath) {
        return std::filesystem::exists(resolvePath(filePath));
    }

    std::vector<uint8_t> FileSystem::readFile(const std::filesystem::path& filePath) {
        std::filesystem::path resolvedPath = resolvePath(filePath);
        std::ifstream in{resolvedPath, std::ios::binary};
        if (!in) {
            AX_CORE_LOG_ERROR("Could not open the file: {0}", resolvedPath.generic_string());
            return {};
        }

        std::vector<uint8_t> buf(std::istreambuf_iterator<char>(in), {});
        return buf;
    }

    std::string FileSystem::readFileStr(const std::filesystem::path& filePath) {
        std::filesystem::path resolvedPath = resolvePath(filePath);
        std::ifstream in{resolvedPath, std::ios::in | std::ios::binary | std::ios::ate};
        if (!in) {
            AX_CORE_LOG_ERROR("Could not open the file: {0}", resolvedPath.generic_string());
            return {};
        }

        const auto size = in.tellg();
        std::string buf(size, '\0');
        in.seekg(0, std::ios::beg);
        in.read(buf.data(), size);
        in.close();
        return buf;
    }

    void FileSystem::writeFile(const std::filesystem::path& filePath, std::vector<uint8_t>& data) {
        std::filesystem::path resolvedPath = resolvePath(filePath);
        std::ofstream out{resolvedPath, std::ios::binary | std::ios::trunc};
        if (!out) {
            AX_CORE_LOG_ERROR("Could not write the file: {0}", resolvedPath.generic_string());
        }
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        out.close();
    }

    std::vector<FileInfo> FileSystem::getDirectory(const std::filesystem::path& folderPath) {
        std::filesystem::path resolvedPath = resolvePath(folderPath);
        std::vector<FileInfo> files{};

        if (!exists(resolvedPath)) {
            AX_CORE_LOG_WARN("Could not get the directory: {0}", resolvedPath.generic_string());
            return files;
        }

        for (const auto& entry : std::filesystem::directory_iterator(resolvedPath)) {
            FileInfo fileInfo;
            fileInfo.name = entry.path().filename().string();
            fileInfo.isDirectory = entry.is_directory();
            fileInfo.size = entry.is_regular_file() ? entry.file_size() : 0;
            files.push_back(fileInfo);
        }
        return files;
    }

    void FileSystem::createDirectory(const std::filesystem::path& folderPath) {
        std::filesystem::create_directory(resolvePath(folderPath));
    }

    void FileSystem::setWorkingDirectory(const std::filesystem::path& folderPath) {
        std::filesystem::path resolvedPath = resolvePath(folderPath);
        if (std::filesystem::exists(resolvedPath) && std::filesystem::is_directory(resolvedPath)) {
            workingDirectory = std::filesystem::absolute(resolvedPath);
            std::filesystem::current_path(workingDirectory);
        } else {
            AX_CORE_LOG_ERROR("The specified path does not exist or is not a directory: {}", resolvedPath.generic_string());
        }
    }
} // namespace Axiom
