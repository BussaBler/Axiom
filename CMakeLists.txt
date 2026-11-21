cmake_minimum_required(VERSION 3.20)
project(Axiom VERSION 1.0 LANGUAGES CXX)

# shaderc config
set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Vendor/SpirvCross
${CMAKE_BINARY_DIR}/Vendor/SpirvCross)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)

if (NOT EXISTS Vendor/Shaderc/third_party/spirv-tools/CMakeLists.txt)
  message(STATUS "[VENDOR] Syncing Shaderc dependencies...")
  find_package(Python3 REQUIRED)
  execute_process(COMMAND ${_Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/Shaderc/utils/git-sync-deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/Shaderc
    RESULT_VARIABLE SYNC_RESULT
  )
  if (NOT SYNC_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to sync Shaderc dependencies")
  endif()
endif()

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/Shaderc
  ${CMAKE_BINARY_DIR}/Vendor/Shaderc
)
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/AxImageLoader
  ${CMAKE_BINARY_DIR}/Vendor/AxImageLoader
)
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/AxModelLoader
  ${CMAKE_BINARY_DIR}/Vendor/AxModelLoader
)

# axiom core config
if (WIN32)
  set(PLATFORM_DEFINE "AX_PLATFORM_WINDOWS")
  file(GLOB_RECURSE PLATFORM_SOURCES "Source/Platform/Windows/*.cpp")
elseif (APPLE)
  set(PLATFORM_DEFINE "AX_PLATFORM_MACOS")
  file(GLOB_RECURSE PLATFORM_SOURCES "Source/Platform/MacOS/*.cpp")
  file(GLOB_RECURSE OBJCXX_SOURCES "Source/Platform/MacOS/*.mm")
  set_source_files_properties(${OBJCXX_SOURCES} PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
    COMPILE_FLAGS "-fobjc-arc"
  )
  list(APPEND PLATFORM_SOURCES ${OBJCXX_SOURCES})
  find_library(METAL_FRAMEWORK Metal)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  find_library(QUARTZ_FRAMEWORK QuartzCore)
  find_library(APPKIT_FRAMEWORK AppKit)
  set(MAC_FRAMEWORKS
    ${METAL_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${QUARTZ_FRAMEWORK}
    ${APPKIT_FRAMEWORK}
  )
elseif (UNIX)
  set(PLATFORM_DEFINE "AX_PLATFORM_LINUX")
  file(GLOB_RECURSE PLATFORM_SOURCES "Source/Platform/Linux/*.cpp")
endif()

file(GLOB_RECURSE CORE_SOURCES
    "Source/Core/*.cpp" "Source/Core/*.h"
    "Source/Math/*.cpp" "Source/Math/*.h"
    "Source/Utils/*.cpp" "Source/Utils/*.h"
)
list(APPEND CORE_SOURCES ${PLATFORM_SOURCES})
add_library(AxiomCore STATIC ${CORE_SOURCES})

target_compile_definitions(AxiomCore PUBLIC ${PLATFORM_DEFINE})
target_compile_definitions(AxiomCore PUBLIC
  AX_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_compile_definitions(AxiomCore PUBLIC
  $<$<CONFIG:Debug>:AX_DEBUG>
  $<$<CONFIG:Debug>:AX_ENABLE_ASSERTS>
  $<$<NOT:$<CONFIG:Debug>>:AX_RELEASE>
)
target_include_directories(AxiomCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
  Vendor
)
if (APPLE)
  target_link_libraries(AxiomCore PUBLIC ${MAC_FRAMEWORKS})
endif()
target_precompile_headers(AxiomCore PRIVATE Source/axpch.h)

# shader baker config
add_executable(AxiomShaderBaker
  ShaderBaker/ShaderBaker.cpp
  "Source/Renderer/ShaderCompiler.cpp"
  "Source/Renderer/ShaderIncluder.cpp"
)
target_link_libraries(AxiomShaderBaker PRIVATE AxiomCore shaderc)
target_include_directories(AxiomShaderBaker PRIVATE
  Vendor/Shaderc/libshaderc/include
)
set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Packages/Shaders")
set(GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Generated")
set(GENERATED_SHADER_HEADER "${GENERATED_INCLUDE_DIR}/BuiltInShaders.h")
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})
file(GLOB BUILTIN_SHADERS "${SHADERS_SRC_DIR}/BuiltIn.*.axs")
file(GLOB SHADER_INCLUDES  "${SHADERS_SRC_DIR}/Includes/*.axsinc")
add_custom_command(
  OUTPUT ${GENERATED_SHADER_HEADER}
  COMMAND $<TARGET_FILE:AxiomShaderBaker> ${SHADERS_SRC_DIR} ${GENERATED_SHADER_HEADER}
  DEPENDS AxiomShaderBaker ${BUILTIN_SHADERS} ${SHADER_INCLUDES}
  COMMENT "Baking Core Shaders"
)

# header tool config
add_executable(AxiomHeaderTool HeaderTool/HeaderTool.cpp)
set(COMPONENTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source/ECS/Components")
set(GENERATED_REFLECTION "${GENERATED_INCLUDE_DIR}/ReflectedComponents.cpp")
file(GLOB COMPONENT_HEADERS CONFIGURE_DEPENDS "${COMPONENTS_DIR}/*.h")
add_custom_command(
  OUTPUT ${GENERATED_REFLECTION}
  COMMAND $<TARGET_FILE:AxiomHeaderTool> ${COMPONENTS_DIR} ${GENERATED_REFLECTION} ECS/Components
  DEPENDS AxiomHeaderTool ${COMPONENT_HEADERS}
  COMMENT "Generating component reflection"
)

# axiom engine config
file(GLOB_RECURSE ENGINE_SOURCES "Source/*.cpp" "Source/*.h" "Source/*.mm")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Source/Core/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Source/Math/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Source/Utils/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Source/Platform/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Source/Renderer/Backend/.*")

if (AX_RENDERER STREQUAL "vulkan")
  file(GLOB_RECURSE RENDERER_SOURCES
    "Source/Renderer/Backend/Vulkan/*.cpp" "Source/Renderer/Backend/Vulkan/*.h"
  )
elseif (AX_RENDERER STREQUAL "metal")
  file(GLOB_RECURSE RENDERER_SOURCES
    "Source/Renderer/Backend/Metal/*.cpp" "Source/Renderer/Backend/Metal/*.h"
  )
elseif (AX_RENDERER STREQUAL "dx12")
  file(GLOB_RECURSE RENDERER_SOURCES
    "Source/Renderer/Backend/DX12/*.cpp" "Source/Renderer/Backend/DX12/*.h"
  )
endif()
list(APPEND ENGINE_SOURCES ${RENDERER_SOURCES})

add_library(AxiomEngine STATIC ${ENGINE_SOURCES})
target_sources(AxiomEngine PRIVATE ${GENERATED_SHADER_HEADER})
target_sources(AxiomEngine PRIVATE ${GENERATED_REFLECTION})
target_include_directories(AxiomEngine PUBLIC
  ${GENERATED_INCLUDE_DIR}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
  Vendor/Vulkan/Include
  Vendor/AxImageLoader/Include
  Vendor/AxModelLoader/Include
  Vendor/SpirvCross
  Vendor/Shaderc/libshaderc/include
)
target_link_libraries(AxiomEngine PUBLIC
  AxiomCore
  AxImageLoader
  AxModelLoader
  spirv-cross-core
  spirv-cross-glsl
  spirv-cross-msl
  shaderc_combined
)

if (AX_RENDERER STREQUAL "vulkan")
  find_package(Vulkan REQUIRED)
  target_link_libraries(AxiomEngine PUBLIC Vulkan::Vulkan)
elseif (AX_RENDERER STREQUAL "dx12")
  target_link_libraries(AxiomEngine PUBLIC d3d12.lib dxgi.lib dxguid.lib)
endif()

target_precompile_headers(AxiomEngine PRIVATE Source/axpch.h)
