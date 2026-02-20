Import('baseEnv', 'debugEnv', 'releaseEnv', 'buildInfo')
import os
from SCons.Script import Dir, Return

platform = buildInfo['platform']
architecture = buildInfo['architecture'] 
compiler = buildInfo['compiler']
config = buildInfo['config']
vulkanSdk = buildInfo['vulkanSdk']
vsproj = buildInfo['vsproj']

currentEnv = debugEnv if config.lower() == 'debug' else releaseEnv
configName = config.capitalize()

objPrefix = f'Bin-Int/{platform}-{architecture}/{configName}/'
currentEnv['OBJPREFIX'] = objPrefix

srcDir = Dir('../../Axiom/Source')
allCppFiles = srcDir.glob('Axiom/**/*.cpp')
allCppFiles += srcDir.glob('Axiom/Renderer/**/*.cpp')
allCppFiles += srcDir.glob('Axiom/Renderer/Data/**/*.cpp')

axiomSources = [f for f in allCppFiles if '/Platform/' not in str(f) and '\\Platform\\' not in str(f)]

axiomSources += srcDir.glob('Platform/Vulkan/*.cpp')
axiomSources += srcDir.glob('Platform/Vulkan/Shader/*.cpp')

def configurePlatformSpecific(env, platformName, compilerType):
    # configure platform-specific settings
    
    if compilerType == 'msvc':
        env.Append(CXXFLAGS=['/std:c++20'])
    else:
        env.Append(CXXFLAGS=['-std=c++20'])
    
    if platformName.startswith('windows'):
        env.Append(CPPDEFINES=['AX_PLATFORM_WINDOWS'])
        return srcDir.glob('Platform/Windows/*.cpp')
    elif platformName.startswith('linux'):
        env.Append(CPPDEFINES=['AX_PLATFORM_LINUX'])
        return srcDir.glob('Platform/Linux/*.cpp')
    elif platformName.startswith('darwin'):
        env.Append(CPPDEFINES=['AX_PLATFORM_MACOS'])
        return srcDir.glob('Platform/MacOS/*.cpp')
    else:
        return []

platformSources = configurePlatformSpecific(currentEnv, platform, compiler)
axiomSources += platformSources

currentEnv.Append(CPPPATH=[
    '../../Axiom/Source',
    '../../Axiom/Source/Axiom',
    '../../Axiom/Vendor/glm',
    os.path.join(vulkanSdk, 'Include'),
    '../../Axiom/Vendor/ImGui',
    '../../Axiom/Vendor/stb',
])

if compiler == 'msvc':
    currentEnv.Append(LIBPATH=[os.path.join(vulkanSdk, 'Lib')])
    shadercLib = 'shaderc_combined' + ('d' if config.lower() == 'debug' else '')
    currentEnv.Append(LIBS=[shadercLib])
else:  # GCC or Clang
    currentEnv.Append(LIBPATH=[os.path.join(vulkanSdk, 'lib')])
    currentEnv.Append(LIBS=['shaderc_combined'])

currentEnv['OBJPREFIX'] = f'../../../Bin-Int/{platform}-{architecture}/{configName}/'

axiomLib = currentEnv.StaticLibrary('Axiom', axiomSources)

axiomProject = None
if vsproj:
    axiomSources = srcDir.glob('Axiom/**/*.cpp')
    axiomSources += srcDir.glob('Axiom/**/**/*.cpp')
    axiomSources += srcDir.glob('Axiom/**/**/**/*.cpp')
    axiomSources += srcDir.glob('Platform/**/*.cpp')
    axiomSources += srcDir.glob('Platform/**/**/*.cpp')
    axiomSources += srcDir.glob('*.cpp')

    axiomHeaders = srcDir.glob('Axiom/**/*.h')
    axiomHeaders += srcDir.glob('Axiom/**/**/*.h')
    axiomHeaders += srcDir.glob('Axiom/**/**/**/*.h')
    axiomHeaders += srcDir.glob('Platform/**/*.h')
    axiomHeaders += srcDir.glob('Platform/**/**/*.h')
    axiomHeaders += srcDir.glob('*.h')

    currentEnv['CPPPATH'] = [Dir(path) for path in currentEnv['CPPPATH']]

    axiomProject = currentEnv.MSVSProject(
        target='../../Axiom/Axiom' + currentEnv['MSVSPROJECTSUFFIX'],
        srcs=[str(f) for f in axiomSources],
        incs=[str(f) for f in axiomHeaders],
        buildtarget=axiomLib,
        variant=[f'{configName}|x64'],
        auto_build_solution=0,
    )


Return('axiomLib', 'axiomProject')