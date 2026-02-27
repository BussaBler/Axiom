Import('baseEnv', 'debugEnv', 'releaseEnv', 'buildInfo')

import os
from SCons.Script import Dir, Return

platform = buildInfo['platform']
architecture = buildInfo['architecture']
compiler = buildInfo['compiler']
config = buildInfo['config']
configName = config.capitalize()
vsproj = buildInfo['vsproj']

localEnv = (debugEnv if config.lower() == 'debug' else releaseEnv).Clone()

def configurePlatform(env, platformName):
    platformMap = {
        'windows': {'define': ['AX_PLATFORM_WINDOWS', 'VK_USE_PLATFORM_WIN32_KHR'], 'path': 'Source/Platform/Windows/*.cpp'},
        'linux': {'define': ['AX_PLATFORM_LINUX', 'VK_USE_PLATFORM_XLIB_KHR'], 'path': 'Source/Platform/Linux/*.cpp'},
        'darwin': {'define': 'AX_PLATFORM_MACOS', 'path': 'Source/Platform/MacOS/*.cpp'}
    }

    key = next((k for k in platformMap if platformName.startswith(k)), None)
    
    if key:
        env.Append(CPPDEFINES=platformMap[key]['define'])
        return Glob(platformMap[key]['path'])
    return []

coreSources = Glob('Source/*.cpp')
coreSources += Glob('Source/*/*.cpp')
# TODO: temp solution to add platform-specific code, should be changed to add more renderer backends
vulkanSources = Glob('Source/Platform/Vulkan/*.cpp')

sources = coreSources + vulkanSources + configurePlatform(localEnv, platform)

localEnv.Append(CPPPATH=[
    Dir('#/Axiom/Source'),
])

axiomLib = localEnv.StaticLibrary(f'#/Bin/{configName}/Axiom/Axiom', sources)

axiomProject = None
if vsproj:
    axiomSources = Glob('Source/*.cpp') + Glob('Source/*/*.cpp') + Glob('Source/*/*/*.cpp')
    axiomHeaders = Glob('Source/*.h') + Glob('Source/*/*.h') + Glob('Source/*/*/*.h')
    localEnv['CPPPATH'] = [Dir(path) for path in localEnv['CPPPATH']]

    axiomProject = localEnv.MSVSProject(
        target='#/Axiom/Axiom' + localEnv['MSVSPROJECTSUFFIX'],
        srcs=[str(f) for f in axiomSources],
        incs=[str(f) for f in axiomHeaders],
        buildtarget=axiomLib,
        variant=[f'{configName}|x64'],
        auto_build_solution=0,
    )

Return('axiomLib', 'axiomProject')