Import('base_env', 'debug_env', 'release_env', 'build_info')
import os
from SCons.Script import Dir

# ============================================================================
# Build Configuration
# ============================================================================

platform = build_info['platform']
architecture = build_info['architecture'] 
compiler = build_info['compiler']
config = build_info['config']
vulkan_sdk = build_info['vulkan_sdk']

# Select the appropriate environment
current_env = debug_env if config.lower() == 'debug' else release_env
config_name = config.capitalize()

# Set output directory for object files
obj_prefix = f'/Bin-Int/{platform}-{architecture}/{config_name}/'
current_env['OBJPREFIX'] = obj_prefix

# ============================================================================
# ImGui Library Build
# ============================================================================

imgui_src_dir = Dir('../../Axiom/Vendor/ImGui')
imgui_sources = imgui_src_dir.glob('*.cpp')
current_env.Append(CPPPATH=[imgui_src_dir])
imgui_lib = current_env.StaticLibrary('ImGui', imgui_sources)

# ============================================================================
# Axiom Library Source Files
# ============================================================================

src_dir = Dir('../../Axiom/Source')
all_cpp_files = src_dir.glob('Axiom/**/*.cpp')

# Exclude platform-specific files initially
axiom_sources = [f for f in all_cpp_files if '/Platform/' not in str(f) and '\\Platform\\' not in str(f)]

# Add Vulkan platform files (common across platforms)
axiom_sources += src_dir.glob('Platform/Vulkan/*.cpp')

# ============================================================================
# Platform-Specific Configuration
# ============================================================================

def configure_platform_specific(env, platform_name, compiler_type, vulkan_path):
    """Configure platform-specific settings"""
    
    # Add C++20 standard
    if compiler_type == 'msvc':
        env.Append(CXXFLAGS=['/std:c++20'])
    else:
        env.Append(CXXFLAGS=['-std=c++20'])
    
    # Platform-specific defines and sources
    if platform_name.startswith('windows'):
        env.Append(CPPDEFINES=['AX_PLATFORM_WINDOWS'])
        return src_dir.glob('Platform/Windows/*.cpp')
    elif platform_name.startswith('linux'):
        env.Append(CPPDEFINES=['AX_PLATFORM_LINUX'])
        return src_dir.glob('Platform/Linux/*.cpp')
    elif platform_name.startswith('darwin'):
        env.Append(CPPDEFINES=['AX_PLATFORM_MACOS'])
        return src_dir.glob('Platform/MacOS/*.cpp')
    else:
        return []

# Add platform-specific sources
platform_sources = configure_platform_specific(current_env, platform, compiler, vulkan_sdk)
axiom_sources += platform_sources

# ============================================================================
# Include Paths and Libraries
# ============================================================================

current_env.Append(CPPPATH=[
    '../../Axiom/Source',
    '../../Axiom/Source/Axiom',
    '../../Axiom/Vendor/glm',
    os.path.join(vulkan_sdk, 'Include'),
    '../../Axiom/Vendor/ImGui',
])

# Platform and compiler specific library configuration
if compiler == 'msvc':
    current_env.Append(LIBPATH=[os.path.join(vulkan_sdk, 'Lib')])
    shaderc_lib = 'shaderc_combined' + ('d' if config.lower() == 'debug' else '')
    current_env.Append(LIBS=[shaderc_lib])
else:  # GCC or Clang
    current_env.Append(LIBS=['vulkan-1', 'shaderc_combined'])

# Update object file prefix
current_env['OBJPREFIX'] = f'../../../Bin-Int/{platform}-{architecture}/{config_name}/'

# Build the Axiom library
axiom_lib = current_env.StaticLibrary('Axiom', axiom_sources)