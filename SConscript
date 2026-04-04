Import('baseEnv', 'debugEnv', 'releaseEnv', 'buildInfo')

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
        'windows': {'define': 'AX_PLATFORM_WINDOWS', 'path': ['Source/Platform/Windows/*.cpp']},
        'linux': {'define': 'AX_PLATFORM_LINUX', 'path': ['Source/Platform/Linux/*.cpp']},
        'macos': {'define': 'AX_PLATFORM_MACOS', 'path': ['Source/Platform/MacOS/*.cpp', 'Source/Platform/MacOS/*.mm']},
    }

    key = next((k for k in platformMap if platformName.startswith(k)), None)
    
    if key:
        env.Append(CPPDEFINES=[platformMap[key]['define']])

        pathGlobs = []
        for path in platformMap[key]['path']:
            pathGlobs.extend(Glob(path))

        return pathGlobs
    return []

coreSources = Glob('Source/*.cpp')
coreSources += Glob('Source/*/*.cpp')
coreSources += Glob('Source/UI/MSDFGen/core/*.cpp')
# TODO: temp solution to add platform-specific code, should be changed to add more renderer backends
vulkanSources = Glob('Source/Platform/Vulkan/*.cpp')

sources = coreSources + vulkanSources
platformSpecificSources = configurePlatform(localEnv, platform)

for source in platformSpecificSources:
    if source not in sources:
        sources.append(source)

localEnv.Append(CPPPATH=[
    Dir('#/Axiom/Source'),
    Dir('#/Vendor/AxImageLoader/Include'),
])

axiomLib = localEnv.StaticLibrary(f'#/Bin/{configName}/Axiom/Axiom', sources)

axiomProject = None
if vsproj:
    axiomSources = Glob('Source/*.cpp') + Glob('Source/*/*.cpp') + Glob('Source/*/*/*.cpp') + Glob('Source/*/*/*/*.cpp') + Glob('Source/*/*/*/*/*.cpp')
    axiomHeaders = Glob('Source/*.h') + Glob('Source/*/*.h') + Glob('Source/*/*/*.h') + Glob('Source/*/*/*/*.h') + Glob('Source/*/*/*/*/*.h')
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