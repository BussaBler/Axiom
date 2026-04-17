#include "AssetManager.h"
#include "AxImageLoader.h"
#include "Core/Application.h"

namespace Axiom {
    std::unordered_map<UUID, std::shared_ptr<Asset>> AssetManager::assets;
    std::unordered_map<std::string, UUID> AssetManager::assetHandles;

    UUID AssetManager::loadTexture(const std::filesystem::path& path) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        UUID handle = UUID();

        auto imageResult = AxImageLoader::loadImage(path, 4);

        if (imageResult.has_value()) {
            Texture::CreateInfo createInfo = {.width = imageResult->width,
                                              .height = imageResult->height,
                                              .mipLevels = 1,
                                              .arrayLayers = 1,
                                              .format = Format::R8G8B8A8Unorm,
                                              .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                              .aspect = TextureAspect::Color,
                                              .initialState = TextureState::Undefined,
                                              .memoryUsage = MemoryUsage::GPUOnly};

            std::shared_ptr<Texture> texture = Application::getRenderer()->createTexture(createInfo);

            Buffer::CreateInfo stagingBufferCreateInfo = {
                .size = imageResult->data.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);
            stagingBuffer->setData(imageResult->data.data(), imageResult->data.size());

            std::unique_ptr<CommandBuffer> commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
            commandBuffer->copyBufferToTexture(stagingBuffer.get(), texture.get(), imageResult->width, imageResult->height);
            Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

            assets[handle] = std::make_shared<TextureAsset>(handle, path.filename().string(), texture);
            assetHandles[cacheString] = handle;
            return handle;
        }

        AX_CORE_LOG_ERROR("Failed to load texture: {}", imageResult.error());

        return 0;
    }

    UUID AssetManager::loadShader(const std::filesystem::path& path) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        UUID handle = UUID();

        auto source = FileSystem::readFileStr(path);

        size_t vertexPos = source.find("#type vertex");
        size_t fragmentPos = source.find("#type fragment");

        std::string vertexSource = source.substr(vertexPos + 13, fragmentPos - (vertexPos + 13));
        std::string fragmentSource = source.substr(fragmentPos + 15, std::string::npos);

        std::shared_ptr<Shader> shader = Application::getRenderer()->createShader(vertexSource, fragmentSource);
        assets[handle] = std::make_shared<ShaderAsset>(handle, path.filename().string(), shader);
        assetHandles[cacheString] = handle;
        return handle;
    }
} // namespace Axiom
