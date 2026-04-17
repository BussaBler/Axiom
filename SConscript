Import('baseEnv', 'debugEnv', 'releaseEnv', 'buildInfo', 'axImageLoaderLib', 'spirvCrossLib')

from SCons.Script import Dir, Return

platform = buildInfo['platform']
architecture = buildInfo['architecture']
compiler = buildInfo['compiler']
config = buildInfo['config']
configName = config.capitalize()
vsproj = buildInfo['vsproj']
renderer = buildInfo['renderer']

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

if renderer.lower() == 'vulkan':
    rendererSources = Glob('Source/Platform/Vulkan/*.cpp')
elif renderer.lower() == 'metal':
    rendererSources = Glob('Source/Platform/Metal/*.cpp')
    rendererSources += Glob('Source/Platform/Metal/*.mm')
elif renderer.lower() == 'dx12': 
    rendererSources = Glob('Source/Platform/DX12/*.cpp')

sources = coreSources + rendererSources
platformSpecificSources = configurePlatform(localEnv, platform)

for source in platformSpecificSources:
    if source not in sources:
        sources.append(source)

localEnv.Append(
    CPPPATH=[
        Dir('#/Axiom/Source'),
        Dir('#/Vendor/AxImageLoader/Include'),
        Dir('#/Vendor/SpirvCross'),
    ],
    LIBPATH=[
        Dir(f'#/Bin/{configName}/AxImageLoader'),
        Dir(f'#/Bin/{configName}/SpirvCross'),
    ]
)
localEnv.Prepend(LIBS=[axImageLoaderLib, spirvCrossLib])

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