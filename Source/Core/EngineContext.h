#pragma once

#include "Asset/AssetManager.h"
#include "Core/TaskManager.h"
#include "Core/Window.h"
#include "ECS/ComponentRegistry.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/SceneRenderer.h"
#include "UI/UIRenderer.h"

namespace Axiom {
    struct EngineContext {
        Window* window = nullptr;
        TaskManager* taskManager = nullptr;
        RenderHardwareInterface* rhi = nullptr;
        AssetManager* assetManager = nullptr;
        ComponentRegistry* componentRegistry = nullptr;
        UIRenderer* uiRenderer = nullptr;
        SceneRenderer* sceneRenderer = nullptr;
    };
} // namespace Axiom
