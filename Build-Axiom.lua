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
    return os.rename(path, path) ~= nil
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
    local install_cmd
    if is_windows then
        install_cmd = string.format('.\\%s --root "%s" --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.debug',
                                   installer_name, install_path)
    else
        install_cmd = string.format('tar -xJf "%s" -C "%s"', installer_name, install_path)
    end

    local install_ok = os.execute(install_cmd)
    assert(install_ok == true, "Error: Failed to install Vulkan SDK")

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

    include "vendor/GLFW/Build-GLFW.lua"

    project "Axiom"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("Bin/" .. OutputDir .. "/%{prj.name}")
        objdir    ("Bin-Int/" .. OutputDir .. "/%{prj.name}")

        pchheader "axpch.h"
        pchsource "Axiom/src/axpch.cpp"

        files {
            "src/**.h",
            "src/**.cpp",
        }

        vpaths {
            ["Source Files"] = "src/**"
        }

        IncludeDir = {
            glm    = "vendor/glm",
            glfw   = "vendor/GLFW/include",
            vulkan = ("vendor/VulkanSDK/%s/Include"):format(VulkanSDKVersion),
        }

        includedirs {
            "src",
            "src/Axiom",
            "%{IncludeDir.glm}",
            "%{IncludeDir.glfw}",
            "%{IncludeDir.vulkan}",
        }

        defines { "GLFW_INCLUDE_NONE" }
        libdirs { "vendor/VulkanSDK/" .. VulkanSDKVersion .. "/Lib" }
        links { "GLFW", "vulkan-1" }

        filter "configurations:Debug"
            defines     { "DEBUG", "AX_ENABLE_ASSERTS" }
            runtime     "Debug"
            symbols     "On"
            links       { "shaderc_combinedd" }

        filter "configurations:Release"
            defines     { "RELEASE" }
            runtime     "Release"
            optimize    "On"
            symbols     "On"
            links       { "shaderc_combined" }
end

main()
