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
    IncludeDir["vulkan"] = "vendor/VulkanSDK/1.4.313.0/Include"
    IncludeDir["spdlog"] = "vendor/spdlog"

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
        --"vendor/glfw-3.4/lib",
        "vendor/VulkanSDK/1.4.313.0/Lib"
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
