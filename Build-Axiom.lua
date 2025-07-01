local VulkanLatestURL = "https://vulkan.lunarg.com/sdk/latest/windows.txt"
local VulkanSDKBaseURL = "https://sdk.lunarg.com/sdk/download/"
local VulkanSDKDir = "vendor/VulkanSDK"

-- Helper to run shell commands and capture output
local function os_capture(cmd)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    return s
end

local function get_latest_vulkan_version()
    local version = os_capture('curl -s "' .. VulkanLatestURL .. '"'):gsub("%s+", "")
    if version == "" then
        error("Failed to get Vulkan latest version")
    end
    return version
end

local function vulkan_sdk_installed(version)
    local path = VulkanSDKDir .. "/" .. version
    local ok = os.rename(path, path)
    return ok ~= nil
end

local function clear_vulkan_sdk_dir()
    if package.config:sub(1,1) == '\\' then
        os.execute('rmdir /S /Q "' .. VulkanSDKDir .. '"')
    else
        os.execute('rm -rf "' .. VulkanSDKDir .. '"')
    end
end

local function install_vulkan_sdk(version)
    local sdk_url = VulkanSDKBaseURL .. version .. "/windows/VulkanSDK-" .. version .. "-Installer.exe"
    local installer = VulkanSDKDir .. "/VulkanSDKInstaller.exe"
    os.execute('mkdir "' .. VulkanSDKDir .. '"')
    os.execute('curl -L -o "' .. installer .. '" "' .. sdk_url .. '"')
    os.execute('"' .. installer .. '" /S /D=' .. VulkanSDKDir .. '/' .. version)
end

local VulkanLatestVersion = get_latest_vulkan_version()
if not vulkan_sdk_installed(VulkanLatestVersion) then
    clear_vulkan_sdk_dir()
    install_vulkan_sdk(VulkanLatestVersion)
end

VulkanSDKVersion = VulkanLatestVersion

include "vendor/GLFW/Build-GLFW.lua"

project "Axiom"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("Bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("Bin-Int/" .. OutputDir .. "/%{prj.name}")

    pchheader "axpch.h"
    pchsource "Axiom/src/axpch.cpp"

    files {
        "src/**.h",
        "src/**.cpp",
    }

    vpaths {
        ["src/*"] = "src/**"
    }

    IncludeDir = {}
    IncludeDir["glm"] = "vendor/glm"
    IncludeDir["glfw"] = "vendor/GLFW/include"
    IncludeDir["vulkan"] = "vendor/VulkanSDK/%{VulkanSDKVersion}/Include"

    includedirs {
        "src",
        "src/Axiom",
        "%{IncludeDir.glm}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.vulkan}",
    }

    defines {
        "GLFW_INCLUDE_NONE"
    }

    libdirs {
        "vendor/VulkanSDK/Lib"
    }

    links {
        "GLFW",
        "vulkan-1",
    }
    filter "configurations:Debug"
        links { "shaderc_combinedd" }

    filter "configurations:Release"
        links { "shaderc_combined" }

    filter "configurations:Debug"
        defines { "DEBUG", "AX_ENABLE_ASSERTS" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"
