#pragma once

#include "Core/Log.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/Texture.h"

#include <any>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Axiom {
    class RenderGraph;

    struct RGTextureHandle {
        uint32_t id = 0xFFFFFFFF;
        inline bool isValid() const { return id != 0xFFFFFFFF; }
    };

    struct TextureAccess {
        uint32_t textureId;
        TextureState state;
    };

    struct FrameContext {
        uint32_t frameIndex = 0;
        float deltaTime = 0.0f;
    };

    class PassBuilder {
      public:
        PassBuilder() = default;
        ~PassBuilder() = default;

        RGTextureHandle createTexture(const std::string& name, const Texture::CreateInfo& createInfo);

        void readTexture(RGTextureHandle handle, TextureState textureState);
        void writeTexture(RGTextureHandle handle, TextureState textureState);
        RGTextureHandle importTexture(const std::string& name, Texture* texture);

      private:
        friend class RenderGraph;
        uint32_t currentPassIndex = 0;
        RenderGraph* renderGraph = nullptr;

        std::vector<TextureAccess> currentReads;
        std::vector<TextureAccess> currentWrites;
    };

    class PassResources {
      public:
        PassResources() = default;
        ~PassResources() = default;

        const Texture* getTexture(RGTextureHandle handle) const;

      private:
        friend class RenderGraph;
        std::unordered_map<uint32_t, Texture*> textures;
    };

    class Blackboard {
      public:
        template <typename T> void add(const T& data) { internalData[typeid(T)] = data; }
        template <typename T> const T& get() const {
            if (internalData.find(typeid(T)) == internalData.end()) {
                AX_CORE_LOG_ERROR("Current Render Graph Context has no data of type:", typeid(T).name());
            }
            return std::any_cast<const T&>(internalData.at(typeid(T)));
        }

      private:
        std::unordered_map<std::type_index, std::any> internalData;
    };

    struct RenderPassNode {
        std::string name;
        std::function<void(const PassResources&, CommandBuffer*, const Blackboard&, const FrameContext&)> execute;

        std::vector<TextureAccess> textureReads;
        std::vector<TextureAccess> textureWrites;
    };

    template <typename ExecuteFn, typename PassData>
    concept ValidExecuteFn = std::invocable<ExecuteFn, const PassData&, const PassResources&, CommandBuffer*, const Blackboard&, const FrameContext&>;
    template <typename SetupFn, typename PassData>
    concept ValidSetupFn = std::invocable<SetupFn, PassBuilder&, PassData&, Blackboard&> &&
                           ValidExecuteFn<std::invoke_result_t<SetupFn, PassBuilder&, PassData&, Blackboard&>, PassData>;

    class RenderGraph {
      public:
        RenderGraph() = default;
        ~RenderGraph() = default;

        void begin();
        template <typename PassData, typename SetupFn>
            requires ValidSetupFn<SetupFn, PassData>
        void addPass(const std::string& name, SetupFn setupFn) {
            PassData passData{};
            RenderPassNode passNode = {.name = name};

            PassBuilder builder{};
            builder.renderGraph = this;
            builder.currentPassIndex = static_cast<uint32_t>(passes.size());

            auto executeFn = setupFn(builder, passData, blackboard);

            passNode.execute = [passData, executeFn](const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                                                     const FrameContext& frameCtx) { executeFn(passData, resources, cmd, blackboard, frameCtx); };

            passNode.textureReads = std::move(builder.currentReads);
            passNode.textureWrites = std::move(builder.currentWrites);
            passes.push_back(std::move(passNode));
        }

        void execute(CommandBuffer* commandBuffer, const FrameContext& frameContext);

        RGTextureHandle importTexture(const std::string& name, Texture* texture);

        template <typename T> void addBlackboardData(const T& data) { blackboard.add(data); }
        template <typename T> const T& getBlackboardData() const { return blackboard.get<T>(); }

      private:
        struct VirtualResource {
            std::string name;
            Texture::CreateInfo createInfo;
            Texture* texture = nullptr;
            uint32_t firstPass = 0xFFFFFFFF;
            uint32_t lastPass = 0;
            TextureState currentState = TextureState::Undefined;

            bool isExternal = false;
        };

      private:
        friend class PassBuilder;
        Blackboard blackboard;
        std::vector<RenderPassNode> passes;
        std::vector<VirtualResource> virtualResources;
        std::vector<std::unique_ptr<Texture>> texturesCache;
    };
} // namespace Axiom
