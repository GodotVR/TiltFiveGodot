#!python
import os
import scons_compiledb

opts = Variables([], ARGUMENTS)


VariantDir("build","src", duplicate=False)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

scons_compiledb.enable_with_cmdline(env)

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['d', 'debug', 'r', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'bin/'))
opts.Add(PathVariable('target_name', 'The library name.', 'TiltFiveGodot', PathVariable.PathAccept))

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot-cpp/godot-headers/"
cpp_bindings_path = "godot-cpp/"
cpp_library = "libgodot-cpp"

tilt_five_headers_path = "TiltFiveNDK/include/include"
tilt_five_library_path = "TiltFiveNDK/lib/win64"
tilt_five_library = "TiltFiveNative.dll.if"

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
elif env['platform'] != 'windows':
    print('Only 64 bit windows is currently supported')
    quit()

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Check our platform specifics
# if env['platform'] == "osx":
#     env['target_path'] += 'osx/'
#     cpp_library += '.osx'
#     env.Append(CCFLAGS=['-arch', 'x86_64'])
#     env.Append(CXXFLAGS=['-std=c++17'])
#     env.Append(LINKFLAGS=['-arch', 'x86_64'])
#     if env['target'] in ('debug', 'd'):
#         env.Append(CCFLAGS=['-g', '-O2'])
#     else:
#         env.Append(CCFLAGS=['-g', '-O3'])

# elif env['platform'] in ('x11', 'linux'):
#     env['target_path'] += 'x11/'
#     cpp_library += '.linux'
#     env.Append(CCFLAGS=['-fPIC'])
#     env.Append(CXXFLAGS=['-std=c++17'])
#     if env['target'] in ('debug', 'd'):
#         env.Append(CCFLAGS=['-g3', '-Og'])
#     else:
#         env.Append(CCFLAGS=['-g', '-O3'])

if env['platform'] == "windows":
    env['target_path'] += 'win64/'
    cpp_library += '.windows'
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV=os.environ)

    env.Append(CPPDEFINES=['WIN32', '_WIN32', '_WINDOWS', '_CRT_SECURE_NO_WARNINGS'])
    env.Append(CCFLAGS=['-W3', '-GR'])
    env.Append(CXXFLAGS='/std:c++17')
    if env['target'] in ('debug', 'd'):
        env.Append(CPPDEFINES=['_DEBUG'])
        env.Append(CCFLAGS=['-EHsc', '-MDd', '-ZI'])
        env.Append(LINKFLAGS=['-DEBUG'])
    else:
        env.Append(CPPDEFINES=['NDEBUG'])
        env.Append(CCFLAGS=['-O2', '-EHsc', '-MD'])

if env['target'] in ('debug', 'd'):
    cpp_library += '.debug'
else:
    cpp_library += '.release'

cpp_library += '.' + str(bits)

# make sure our binding library is properly includes
env.Append(CPPPATH=['.', godot_headers_path, cpp_bindings_path + 'include/', cpp_bindings_path + 'include/core/', cpp_bindings_path + 'include/gen/', tilt_five_headers_path])
env.Append(LIBPATH=[cpp_bindings_path + 'bin/', tilt_five_library_path])
env.Append(LIBS=[cpp_library, tilt_five_library])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
sources = Glob('build/*.cpp')
sources += Glob('build/*.c')

print(sources)

library = env.SharedLibrary(target=env['target_path'] + env['target_name'] , source=sources)

Default(library)


# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
