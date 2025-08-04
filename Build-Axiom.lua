local VULKAN_LATEST_URL    = "https://vulkan.lunarg.com/sdk/latest/windows.txt"
local VULKAN_SDK_BASE_URL  = "https://sdk.lunarg.com/sdk/download/"
local VENDOR_DIR           = "vendor"
local VULKAN_SDK_DIR       = VENDOR_DIR .. "/VulkanSDK"
local SCRIPT_PATH          = debug.getinfo(1, "S").source:sub(2)
local SCRIPT_DIR           = SCRIPT_PATH:match("(.*[/\\])") or "./"
local VULKAN_SDK_DIR_ABS   = SCRIPT_DIR .. VULKAN_SDK_DIR

local is_windows = (package.config:sub(1,1) == "\\")

local function os_capture(cmd)
    local handle = assert(io.popen(cmd, "r"))
    local result = assert(handle:read("*a"))
    handle:close()
    return result
end

local function get_latest_vulkan_version()
    local version = os_capture(string.format('curl -s "%s"', VULKAN_LATEST_URL)):gsub("%s+", "")
    assert(version ~= "", "Error: Unable to fetch the latest Vulkan SDK version")
    return version
end


local function vulkan_sdk_installed(version)
    local path = VULKAN_SDK_DIR .. "/" .. version
    local ret = os.rename(path, path) == true
    return ret
end


local function ensure_dir(path)
    if is_windows then
        os.execute(string.format('if not exist "%s" mkdir "%s"', path, path))
    else
        os.execute(string.format('mkdir -p "%s"', path))
    end
end


local function clear_old_sdk()
    if is_windows then
        os.execute(string.format('rmdir /S /Q "%s"', VULKAN_SDK_DIR))
    else
        os.execute(string.format('rm -rf "%s"', VULKAN_SDK_DIR))
    end
end


local function build_sdk_url(version)
    if is_windows then
        return VULKAN_SDK_BASE_URL .. version .. "/windows/vulkan_sdk.exe", ".exe"
    else
        return VULKAN_SDK_BASE_URL .. version .. "/linux/vulkan_sdk.tar.xz", ".tar.xz"
    end
end

local function install_vulkan_sdk(version)
    print("Installing Vulkan SDK " .. version)

    local sdk_url, file_ext = build_sdk_url(version)
    local installer_name = "VulkanSDKInstaller" .. file_ext

    ensure_dir(VULKAN_SDK_DIR)

    local download_cmd = string.format('curl -L -o "%s" "%s"', installer_name, sdk_url)
    local download_ok = os.execute(download_cmd)
    assert(download_ok == true, "Error: Failed to download Vulkan SDK installer")

    local install_path = VULKAN_SDK_DIR_ABS .. "/" .. version
    ensure_dir(install_path)

    local install_cmd
    if is_windows then
        install_cmd = string.format('.\\%s --root "%s" --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.debug',
                                   installer_name, install_path)
    else
        install_cmd = string.format('tar -xJf "%s" -C "%s"', installer_name, install_path)
    end

    local success, exit_type, exit_code = os.execute(install_cmd)
    assert(success, string.format("Error: Failed to install Vulkan SDK (exit_type=%s, exit_code=%s)", exit_type, exit_code))

    os.remove(installer_name)

    print("Vulkan SDK " .. version .. " installed successfully.")
end


local function main()
    local version = get_latest_vulkan_version()

    if not vulkan_sdk_installed(version) then
        clear_old_sdk()
        install_vulkan_sdk(version)
    else
        print("Vulkan SDK " .. version .. " is already installed.")
    end

    VulkanSDKVersion = version

    include "vendor/imgui/Build-ImGui.lua"

    project "Axiom"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("Bin/" .. OutputDir .. "/%{prj.name}")
        objdir    ("Bin-Int/" .. OutputDir .. "/%{prj.name}")

        pchheader "axpch.h"
        pchsource "Source/axpch.cpp"

        files {
            "Source/**.h",
            "Source/**.cpp",
        }

        removefiles {
            "Source/Platform/Windows/**",
            "Source/Platform/Linux/**",
            "Source/Platform/MacOS/**",
        }

        vpaths {
            ["Source Files"] = "Source/**"
        }

        filter "system:windows"
            systemversion "latest"
            defines { "AX_PLATFORM_WINDOWS" }
            files {
                "Source/Platform/Windows/**.h",
                "Source/Platform/Windows/**.cpp",
            }
        
        filter "system:linux"
            systemversion "latest"
            defines { "AX_PLATFORM_LINUX" }
            files {
                "Source/Platform/Linux/**.h",
                "Source/Platform/Linux/**.cpp",
            }

        filter "system:macosx"
            systemversion "latest"
            defines { "AX_PLATFORM_MACOS" }
            files {
                "Source/Platform/MacOS/**.h",
                "Source/Platform/MacOS/**.cpp",
            }

        filter {}

        IncludeDir = {
            glm    = "vendor/glm",
            vulkan = ("vendor/VulkanSDK/%s/Include"):format(VulkanSDKVersion),
            imgui  = "vendor/imgui",
        }

        includedirs {
            "Source",
            "Source/Axiom",
            "%{IncludeDir.glm}",
            "%{IncludeDir.glfw}",
            "%{IncludeDir.vulkan}",
            "%{IncludeDir.imgui}",
        }

        defines { "GLFW_INCLUDE_NONE" }
        libdirs { "vendor/VulkanSDK/" .. VulkanSDKVersion .. "/Lib" }
        links { "vulkan-1", "ImGui" }

        filter "configurations:Debug"
            defines     { "AX_DEBUG", "AX_ENABLE_ASSERTS" }
            runtime     "Debug"
            symbols     "On"
            links       { "shaderc_combinedd" }

        filter "configurations:Release"
            defines     { "AX_RELEASE" }
            runtime     "Release"
            optimize    "On"
            symbols     "On"
            links       { "shaderc_combined" }
end

main()
