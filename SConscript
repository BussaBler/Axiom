Import('env', 'Debug', 'Release')
import os, sys
from SCons.Script import Glob, Dir, ARGUMENTS

is_msvc = 'msvc' in env['TOOLS']
is_gcc  = 'gcc'  in env['TOOLS'] or 'g++' in env['TOOLS']

config = ARGUMENTS.get('config', 'Debug').lower()
builder = Debug if config == 'debug' else Release

imgui_src_dir = Dir('../Axiom/Vendor/ImGui')
imgui_src = imgui_src_dir.glob('*.cpp')
builder.Append(CPPPATH=[imgui_src_dir])
imgui_lib = builder.StaticLibrary('ImGui', imgui_src)

src_dir = Dir('../Axiom/Source')
all_cpp = src_dir.glob('Axiom/**/*.cpp')
sources = [f for f in all_cpp if '/Platform/' not in str(f) and '\\Platform\\' not in str(f)]
sources += src_dir.glob('Platform/Vulkan/*.cpp')

if is_msvc:
    builder.Append(CPPDEFINES=['AX_PLATFORM_WINDOWS'],
                   CXXFLAGS=['/std:c++20'])
    sources += src_dir.glob('Platform/Windows/*.cpp')
elif is_gcc and sys.platform.startswith('win'):
    builder.Append(CPPDEFINES=['AX_PLATFORM_WINDOWS'],
                   CXXFLAGS=['-std=c++20'])
    sources += src_dir.glob('Platform/Windows/*.cpp')
elif sys.platform.startswith('linux'):
    builder.Append(CPPDEFINES=['AX_PLATFORM_LINUX'])
    builder.Append(CXXFLAGS=['-std=c++20']) 
    sources += src_dir.glob('Platform/Linux/*.cpp')
elif sys.platform.startswith('darwin'):
    builder.Append(CPPDEFINES=['AX_PLATFORM_MACOS'])
    builder.Append(CXXFLAGS=['-std=c++20'])  
    sources += src_dir.glob('Platform/MacOS/*.cpp')

builder.Append(CPPPATH=[
    '../Axiom/Source',
    '../Axiom/Source/Axiom',
    '../Axiom/Vendor/glm',
    os.path.join(os.environ['VULKAN_SDK'], 'Include'),
    '../Axiom/Vendor/ImGui',
])

if is_msvc:
    builder.Append(LIBPATH=[os.path.join(os.environ['VULKAN_SDK'], 'Lib')])
    builder.Append(LIBS=['shaderc_combined' + ('d' if config=='debug' else '')])
elif is_gcc:
    builder.Append(LIBS=['vulkan-1','shaderc_combined'])

axiom_lib = builder.StaticLibrary('Axiom', sources)