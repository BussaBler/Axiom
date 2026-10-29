#include "ShaderIncluder.h"

#include "Core/Log.h"
#include "Utils/FileSystem.h"
#include "shaderc/shaderc.h"

#include <filesystem>
#include <string>

namespace Axiom {
    shaderc_include_result* ShaderIncluder::GetInclude(const char* requestedSource, shaderc_include_type type, const char* requestingSource,
                                                       size_t includeDepth) {
        std::filesystem::path fullPath = FileSystem::resolvePath("axiom://Packages/Shaders/Includes/" + std::string(requestedSource));

        IncludeData* includeData = new IncludeData();
        includeData->sourceName = fullPath.generic_string();
        includeData->content = FileSystem::readFileStr(fullPath);

        if (includeData->content.empty()) {
            AX_CORE_LOG_ERROR("Failed to read include file {} in shader {}", requestedSource, requestingSource);
        }

        shaderc_include_result* includeResult = new shaderc_include_result();
        includeResult->source_name = includeData->sourceName.c_str();
        includeResult->source_name_length = includeData->sourceName.size();
        includeResult->content = includeData->content.c_str();
        includeResult->content_length = includeData->content.size();
        includeResult->user_data = includeData;
        return includeResult;
    }

    void ShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
        if (data) {
            delete static_cast<IncludeData*>(data->user_data);
            delete data;
        }
    }
} // namespace Axiom
