Import('base_env', 'debug_env', 'release_env', 'build_info')
import os
from SCons.Script import Dir, Return

platform = build_info['platform']
architecture = build_info['architecture'] 
compiler = build_info['compiler']
config = build_info['config']
vulkan_sdk = build_info['vulkan_sdk']
vsproj = build_info['vsproj']

current_env = debug_env if config.lower() == 'debug' else release_env
config_name = config.capitalize()

obj_prefix = f'/Bin-Int/{platform}-{architecture}/{config_name}/'
current_env['OBJPREFIX'] = obj_prefix

src_dir = Dir('../../Axiom/Source')
all_cpp_files = src_dir.glob('Axiom/**/*.cpp')
all_cpp_files += src_dir.glob('Axiom/Renderer/**/*.cpp')
all_cpp_files += src_dir.glob('Axiom/Renderer/Data/**/*.cpp')

axiom_sources = [f for f in all_cpp_files if '/Platform/' not in str(f) and '\\Platform\\' not in str(f)]

axiom_sources += src_dir.glob('Platform/Vulkan/*.cpp')
axiom_sources += src_dir.glob('Platform/Vulkan/Shader/*.cpp')

def configure_platform_specific(env, platform_name, compiler_type):
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
platform_sources = configure_platform_specific(current_env, platform, compiler)
axiom_sources += platform_sources

current_env.Append(CPPPATH=[
    '../../Axiom/Source',
    '../../Axiom/Source/Axiom',
    '../../Axiom/Vendor/glm',
    os.path.join(vulkan_sdk, 'Include'),
    '../../Axiom/Vendor/ImGui',
    '../../Axiom/Vendor/stb',
])

if compiler == 'msvc':
    current_env.Append(LIBPATH=[os.path.join(vulkan_sdk, 'Lib')])
    shaderc_lib = 'shaderc_combined' + ('d' if config.lower() == 'debug' else '')
    current_env.Append(LIBS=[shaderc_lib])
else:  # GCC or Clang
    current_env.Append(LIBS=['vulkan-1', 'shaderc_combined'])

current_env['OBJPREFIX'] = f'../../../Bin-Int/{platform}-{architecture}/{config_name}/'

axiom_lib = current_env.StaticLibrary('Axiom', axiom_sources)

axiom_project = None
if vsproj:
    axiom_sources = src_dir.glob('Axiom/**/*.cpp')
    axiom_sources += src_dir.glob('Axiom/**/**/*.cpp')
    axiom_sources += src_dir.glob('Axiom/**/**/**/*.cpp')
    axiom_sources += src_dir.glob('Platform/**/*.cpp')
    axiom_sources += src_dir.glob('Platform/**/**/*.cpp')
    axiom_sources += src_dir.glob('*.cpp')

    axiom_headers = src_dir.glob('Axiom/**/*.h')
    axiom_headers += src_dir.glob('Axiom/**/**/*.h')
    axiom_headers += src_dir.glob('Axiom/**/**/**/*.h')
    axiom_headers += src_dir.glob('Platform/**/*.h')
    axiom_headers += src_dir.glob('Platform/**/**/*.h')
    axiom_headers += src_dir.glob('*.h')

    current_env['CPPPATH'] = [Dir(path) for path in current_env['CPPPATH']]

    axiom_project = current_env.MSVSProject(
        target='../../Axiom/Axiom' + current_env['MSVSPROJECTSUFFIX'],
        srcs=[str(f) for f in axiom_sources],
        incs=[str(f) for f in axiom_headers],
        buildtarget=axiom_lib,
        variant=[f'{config_name}|x64'],
        auto_build_solution=0
    )


Return('axiom_lib', 'axiom_project')