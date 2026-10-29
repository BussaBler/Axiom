#include "TextureRegistry.h"

#include "Asset/AssetManager.h"
#include "Asset/TextureAsset.h"
#include "Asset/UUID.h"
#include "Core/Log.h"
#include "RenderHardwareInterface.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"

namespace Axiom {
    void TextureRegistry::init() {
        for (uint32_t i = 1; i < 1000; i++) {
            freeSlots.push(i);
        }

        ResourceLayout::BindingCreateInfo texturesBindingCreateInfo = {
            .binding = Bindings::BindlessTextureBinding, .type = ResourceType::Texture2D, .stages = ShaderStage::Fragment, .count = 1000};
        ResourceLayout::BindingCreateInfo samplersBindingCreateInfo = {
            .binding = Bindings::BindlessTextureSamplersBinding, .type = ResourceType::Sampler, .stages = ShaderStage::Fragment, .count = 2};
        resourceLayout = rhi->createResourceLayout({texturesBindingCreateInfo, samplersBindingCreateInfo});
        resourceSet = rhi->createResourceSet(resourceLayout.get());

        Sampler* samplersData[] = {rhi->getNearestSampler(), rhi->getLinearSampler()};
        ResourceSet::Binding samplerUpdateBinding = {
            .binding = Bindings::BindlessTextureSamplersBinding,
            .type = ResourceType::Sampler,
            .stages = ShaderStage::Fragment,
            .data = samplersData,
        };
        resourceSet->update({samplerUpdateBinding});
    }

    void TextureRegistry::setFallbackTexture(Texture* texture) {
        updateDescriptorSet(FALLBACK_SLOT, texture);
    }

    uint32_t TextureRegistry::getOrRegister(UUID textureId) {
        if (!textureId.isValid()) {
            return FALLBACK_SLOT;
        }

        auto it = uuidToSlot.find(textureId);
        if (it != uuidToSlot.end()) {
            return it->second;
        }

        if (freeSlots.empty()) {
            AX_CORE_LOG_ERROR("TextureRegistry out of bindless slots!");
            return FALLBACK_SLOT;
        }

        std::shared_ptr<TextureAsset> asset = AssetManager::getAsset<TextureAsset>(textureId);
        if (!asset || !asset->getTexture()) {
            return FALLBACK_SLOT;
        }

        uint32_t availableSlot = freeSlots.front();
        freeSlots.pop();

        updateDescriptorSet(availableSlot, asset->getTexture());

        uuidToSlot[textureId] = availableSlot;
        slotToUuid[availableSlot] = textureId;

        return availableSlot;
    }

    void TextureRegistry::unregister(UUID textureId) {
        auto it = uuidToSlot.find(textureId);
        if (it == uuidToSlot.end()) {
            AX_CORE_LOG_WARN("Tried to unregister a non-registered texture");
            return;
        }

        uint32_t slotToFree = it->second;
        freeSlots.push(slotToFree);

        slotToUuid.erase(slotToFree);
        uuidToSlot.erase(it);
    }

    void TextureRegistry::updateDescriptorSet(uint32_t index, Texture* texture) {
        Texture* textureData[] = {texture};
        ResourceSet::Binding bindingUpdateInfo = {.binding = Bindings::BindlessTextureBinding,
                                                  .type = ResourceType::Texture2D,
                                                  .stages = ShaderStage::Fragment,
                                                  .data = textureData,
                                                  .dataOffset = index};
        resourceSet->update({bindingUpdateInfo});
    }
} // namespace Axiom
