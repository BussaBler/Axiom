#include "axpch.h"

#include "RenderGraph.h"

#include "Core/Log.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/Texture.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace Axiom {
    RGTextureHandle PassBuilder::createTexture(const std::string& name, const Texture::CreateInfo& createInfo) {
        RGTextureHandle handle = {.id = static_cast<uint32_t>(renderGraph->virtualResources.size())};

        RenderGraph::VirtualResource virtualResource = {.name = name, .createInfo = createInfo};
        renderGraph->virtualResources.push_back(virtualResource);

        return handle;
    }

    void PassBuilder::readTexture(RGTextureHandle handle, TextureState textureState) {
        if (!handle.isValid()) {
            return;
        }

        RenderGraph::VirtualResource& virtualResource = renderGraph->virtualResources[handle.id];

        if (virtualResource.firstPass == 0xFFFFFFFF) {
            virtualResource.firstPass = currentPassIndex;
        }

        virtualResource.lastPass = currentPassIndex;
        currentReads.push_back({handle.id, textureState});
    }

    void PassBuilder::writeTexture(RGTextureHandle handle, TextureState textureState) {
        if (!handle.isValid()) {
            return;
        }

        RenderGraph::VirtualResource& virtualResource = renderGraph->virtualResources[handle.id];

        if (virtualResource.firstPass == 0xFFFFFFFF) {
            virtualResource.firstPass = currentPassIndex;
        }

        virtualResource.lastPass = currentPassIndex;
        currentWrites.push_back({handle.id, textureState});
    }

    RGTextureHandle PassBuilder::importTexture(const std::string& name, Texture* texture) {
        for (uint32_t i = 0; i < renderGraph->virtualResources.size(); i++) {
            if (renderGraph->virtualResources[i].isExternal && renderGraph->virtualResources[i].texture == texture) {
                return RGTextureHandle{.id = i};
            }
        }
        return renderGraph->importTexture(name, texture);
    }

    const Texture* PassResources::getTexture(RGTextureHandle handle) const {
        if (!handle.isValid()) {
            AX_CORE_LOG_ERROR("Pass requested an invalid texture");
            return nullptr;
        }
        if (textures.find(handle.id) == textures.end()) {
            AX_CORE_LOG_ERROR("Pass requested an undeclared texture");
            return nullptr;
        }
        return textures.at(handle.id);
    }

    RenderGraph::RenderGraph(RenderHardwareInterface& rhi) : rhi(rhi) {
    }

    void RenderGraph::begin() {
        passes.clear();
        virtualResources.clear();
    }

    uint64_t hashTextureCreateInfo(const Texture::CreateInfo& createInfo) {
        uint64_t hash = 0;
        auto combine = [&hash](auto val) { hash ^= std::hash<decltype(val)>()(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2); };
        combine(createInfo.width);
        combine(createInfo.height);
        combine(static_cast<uint32_t>(createInfo.format));
        combine(static_cast<uint32_t>(createInfo.usage));
        combine(static_cast<uint32_t>(createInfo.type));
        return hash;
    }

    void RenderGraph::execute(CommandBuffer* commandBuffer, const FrameContext& frameCtx, Texture* presentTarget) {
        std::unordered_map<uint64_t, std::vector<Texture*>> freeList;

        for (const auto& uniqueTex : texturesCache) {
            Texture* rawPtr = uniqueTex.get();
            const Texture::CreateInfo& createInfo = rawPtr->getCreateInfo();
            uint64_t hash = hashTextureCreateInfo(createInfo);
            freeList[hash].push_back(rawPtr);
        }

        PassResources passResources{};

        for (uint32_t i = 0; i < passes.size(); i++) {
            RenderPassNode& pass = passes[i];

            for (auto& vResource : virtualResources) {
                if (vResource.firstPass == i && !vResource.isExternal) {
                    uint64_t hash = hashTextureCreateInfo(vResource.createInfo);
                    auto& list = freeList[hash];
                    auto it = std::find_if(list.begin(), list.end(), [&](Texture* t) { return t->getCreateInfo() == vResource.createInfo; });
                    if (it != list.end()) {
                        vResource.texture = *it;
                        vResource.currentState = vResource.texture->getCurrentState();
                        std::swap(*it, list.back());
                        list.pop_back();
                    } else {
                        std::unique_ptr<Texture> newTexture = rhi.createTexture(vResource.createInfo);
                        vResource.texture = newTexture.get();
                        vResource.currentState = vResource.currentState;
                        vResource.texture->setCurrentState(vResource.currentState);
                        texturesCache.push_back(std::move(newTexture));
                    }
                }
            }

            passResources.textures.clear();
            std::vector<Texture::Barrier> passBarriers;

            for (const auto& read : pass.textureReads) {
                VirtualResource& vResource = virtualResources[read.textureId];
                if (vResource.currentState != read.state || vResource.lastAcessWasWrite) {
                    passBarriers.push_back(
                        {.texture = vResource.texture, .oldState = vResource.currentState, .newState = read.state, .aspect = vResource.createInfo.aspect});
                    vResource.currentState = read.state;
                }
                vResource.lastAcessWasWrite = false;
                passResources.textures[read.textureId] = vResource.texture;
            }

            for (const auto& write : pass.textureWrites) {
                VirtualResource& vResource = virtualResources[write.textureId];
                if (vResource.currentState != write.state || vResource.lastAcessWasWrite) {
                    passBarriers.push_back(
                        {.texture = vResource.texture, .oldState = vResource.currentState, .newState = write.state, .aspect = vResource.createInfo.aspect});
                    vResource.currentState = write.state;
                }
                vResource.lastAcessWasWrite = true;
                passResources.textures[write.textureId] = vResource.texture;
            }

            if (!passBarriers.empty()) {
                commandBuffer->pipelineBarrier(passBarriers);
            }

            if (pass.execute) {
                pass.execute(passResources, commandBuffer, blackboard, frameCtx);
            }

            for (auto& vResource : virtualResources) {
                if (vResource.lastPass == i && !vResource.isExternal) {
                    uint64_t hash = hashTextureCreateInfo(vResource.createInfo);
                    freeList[hash].push_back(vResource.texture);
                }
            }
        }

        if (presentTarget) {
            const RGTextureHandle handle = importTexture("Present Target", presentTarget);
            VirtualResource& resource = virtualResources[handle.id];
            if (resource.currentState != TextureState::Present) {
                commandBuffer->pipelineBarrier({{.texture = resource.texture,
                                                 .oldState = resource.currentState,
                                                 .newState = TextureState::Present,
                                                 .aspect = resource.createInfo.aspect}});
                resource.currentState = TextureState::Present;
            }
        }
    }

    RGTextureHandle RenderGraph::importTexture(const std::string& name, Texture* texture) {
        // The same texture imported twice in one frame (e.g. by the FRP and the UI) must share one
        // virtual resource, otherwise each copy tracks its own (stale) state and barriers go wrong.
        for (uint32_t i = 0; i < virtualResources.size(); i++) {
            if (virtualResources[i].isExternal && virtualResources[i].texture == texture) {
                return RGTextureHandle{.id = i};
            }
        }

        RGTextureHandle handle = {.id = static_cast<uint32_t>(virtualResources.size())};

        RenderGraph::VirtualResource virtualResource = {
            .name = name, .createInfo = texture->getCreateInfo(), .texture = texture, .currentState = texture->getCurrentState(), .isExternal = true};
        virtualResources.push_back(virtualResource);

        return handle;
    }
} // namespace Axiom
