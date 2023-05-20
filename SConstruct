#!python
import os
#import scons_compiledb

opts = Variables([], ARGUMENTS)


VariantDir("build/src","src", duplicate=False)
VariantDir("build/T5Integration","T5Integration", duplicate=False)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

#scons_compiledb.enable_with_cmdline(env)

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['d', 'debug', 'r', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'linux', 'osx']))
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
#opts.Add(PathVariable('target_name', 'The library name.', 'TiltFiveGodot', PathVariable.PathAccept))
#opts.Add(EnumVariable('bits', "CPU architecture", '64', ['32', '64']))

# only support 64 at this time..
bits = 64

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['p'] != '':
    env['platform'] = env['p']

if env['platform'] == '':
    env['platform'] = 'windows'

env['target'] =  { 'd' : 'debug', 'debug' : 'debug', 'r' : 'release', 'release' : "release"}[env["target"]]
env['platform_dir'] = { 'windows' : 'win', 'linux' : 'linux', 'osx' : 'osx'}[env["platform"]]
env['build_product_path'] = 'build/bin/'
env['addons_path']  = 'example/addons/tilt-five/${platform_dir}'
env['install_path']  = 'build/install'
env['bits'] = "64"

# Local dependency paths, adapt them to your setup
godot_include_path = ['godot-cpp/' + path + '/' for path in ['godot-headers', 'include', 'include/core', 'include/gen']]
godot_bindings_path = 'godot-cpp/bin/'
godot_cpp_library = env.subst('libgodot-cpp.${platform}.${target}.${bits}')

tilt_five_headers_path = "TiltFiveNDK/include"
tilt_five_library_path = env.subst("TiltFiveNDK/lib/${platform_dir}/x86_64")
t5_integration_path = "T5Integration"
tilt_five_library = "TiltFiveNative.dll.if"

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Check our platform specifics
# if env['platform'] == "osx":
#     env['build_product_path'] += 'osx/'
#     godot_cpp_library += '.osx'
#     env.Append(CCFLAGS=['-arch', 'x86_64'])
#     env.Append(CXXFLAGS=['-std=c++17'])
#     env.Append(LINKFLAGS=['-arch', 'x86_64'])
#     if env['target'] in ('debug', 'd'):
#         env.Append(CCFLAGS=['-g', '-O2'])
#     else:
#         env.Append(CCFLAGS=['-g', '-O3'])


if env['platform'] == "windows":
    env['build_product_path'] += 'win/'
    env['lib_ext'] = 'dll'
    env['t5_shared_lib'] = 'TiltFiveNative'
    env['t5_shared_link_lib'] = 'TiltFiveNative.dll.if'
    env['target_name'] = 'TiltFiveGodot'
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV=os.environ)

    env.Append(CPPDEFINES=['WIN32', '_WIN32', '_WINDOWS', '_CRT_SECURE_NO_WARNINGS'])
    env.Append(CCFLAGS=['-W3', '-GR'])
    env.Append(CXXFLAGS=['/std:c++20', '/Zc:__cplusplus'])
    if env['target'] in ('debug', 'd'):
        env.Append(CPPDEFINES=['_DEBUG'])
        env.Append(CCFLAGS=['-EHsc', '-MDd', '-ZI'])
        env.Append(LINKFLAGS=['-DEBUG'])
    else:
        env.Append(CPPDEFINES=['NDEBUG'])
        env.Append(CCFLAGS=['-O2', '-EHsc', '-MD'])
elif env['platform'] in ('x11', 'linux'):
    env['build_product_path'] += 'linux/'
    env['lib_ext'] = 'so'
    env['t5_shared_lib'] = 'libTiltFiveNative'
    env['t5_shared_link_lib'] = 'libTiltFiveNative'
    env['target_name'] = 'libTiltFiveGodot'
    env.Append(CCFLAGS=['-fPIC'])
    env.Append(CXXFLAGS=['-std=c++2b'])
    env.Append(RPATH=env.Literal('\\$$ORIGIN' ))
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS=['-g3', '-Og'])
    else:
        env.Append(CCFLAGS=['-g', '-O3'])

# make sure our binding library is properly includes
env.Append(CPPPATH=['.'] + godot_include_path + [tilt_five_headers_path])
env.Append(LIBPATH=[godot_bindings_path, tilt_five_library_path])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/', t5_integration_path])
sources = Glob('build/src/*.cpp')
sources += Glob('build/T5Integration/*.cpp')

library = env.SharedLibrary(target=env.subst('${build_product_path}${target_name}') , source=sources, LIBS=[godot_cpp_library, env['t5_shared_link_lib']])

f1 = env.Command(env.subst('$addons_path/${target_name}.${lib_ext}'), library, Copy('$TARGET', '$SOURCE') )
f2 = env.Command(env.subst('$addons_path/${t5_shared_lib}.${lib_ext}'), env.subst('TiltFiveNDK/lib/${platform_dir}/x86_64/${t5_shared_lib}.${lib_ext}'), Copy('$TARGET', '$SOURCE') )

env.Alias('example', [f1, f2])

# We do this in CI now
# zip_target1 = env.Zip('build/install/tilt-five.zip', 'example/addons', ZIPROOT='example')
# zip_target2 = env.Zip('build/install/tilt-five.zip', 'example/LICENSE.txt', ZIPROOT='example')
#
# env.Alias('zip', [zip_target1, zip_target2])

Default(library)



# Generates help for the -h scons option.
Help('''
scons - Build gdnative extension
scons example - Copy extension to example/addons
scons zip - Build zip archive for import into Godot
''')
Help(opts.GenerateHelpText(env))
