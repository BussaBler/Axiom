# Axiom Game Engine

Axiom is a custom, minimal-dependency C++ game engine designed for recreational development

It handles windowing, input, and abstracts the graphics pipeline to support modern rendering APIs. Axiom is designed to be highly modular and is meant to be consumed as a Git submodule in parent applications (such as the Theorem Editor).

## Core Features
* **Cross-Platform Rendering:** Native backend support for **Vulkan** (Windows/Linux), **Metal** (macOS), and **DirectX 12** (Windows).
* **Automated Shader Pipeline:** Dynamically compiles shaders at build-time using integrated Khronos tools.
* **Asset Loading:** Custom wrappers for image and 3D model parsing.
* **Clean Architecture:** Strict separation of engine logic from application logic.

## Dependencies

Axiom prides itself on keeping external dependencies to an absolute minimum. The engine tracks the following dependencies via internal Git submodules:
* **Shaderc & SPIRV-Cross**: For cross-platform shader compilation and translation.
* **AxImageLoader**: Custom image parsing backend.
* **AxModelLoader**: Custom 3D model parsing backend.

## Getting Started (Standalone)

If you are cloning Axiom to modify the engine directly, be sure to clone recursively to pull in the vendor dependencies:

```bash
git clone --recursive git@github.com:BussaBler/Axiom.git
cd Axiom
```

## Integrating Axiom into a Project

Axiom is built with Modern CMake, making it incredibly easy to embed into other applications.

**1. Add as a submodule:**
```bash
mkdir Engine
git submodule add git@github.com:BussaBler/Axiom.git Dir/Axiom
git submodule update --init --recursive
```

**2. Link via CMakeLists.txt:**
In your parent project's `CMakeLists.txt`, simply add the directory and link the library. Axiom will automatically propagate its include directories, definitions, and render API flags to your application.

```cmake
# Add the engine
add_subdirectory(Dir/Axiom)

# Build your app
add_executable(MyApp main.cpp)

# Link the engine
target_link_libraries(MyApp PRIVATE Axiom)
```

### Configuration
Parent projects can control Axiom's behavior before calling `add_subdirectory()` by passing CMake arguments or setting cache variables:
* `-DAX_RENDERER=[vulkan|dx12|metal]`
* `-DCMAKE_BUILD_TYPE=[Debug|Release]`
