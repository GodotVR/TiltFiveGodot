# TiltFiveGodot

TiltFiveGodot is **GDNative** extension for the Godot engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It implements Godot's **ARVRinterface** and has GDNative class called TiltFiveManager for scripts to connect 
glasses and handle connection events.

## Platforms

Currently only Windows 10/11 is supported because that is the only platform supported by Tilt Five. T5 linux support
is supposed to come at some point in the future and support for that platform will revisited when it becomes available. 

## Build

This project uses the same [SCons](https://scons.org/) build system used by Godot project. 

> `scons target=[debug | release]` Build the shared library. Result is in `build\bin`

> `scons example target=[debug | release]` Copy build products to the `example\addons\tilt-five`

> `scons zip target=[debug | release]` Create a zip archive of `example\addons`

Note that currently due to bugs the zip archive is not compatible with godot's import function.

## Using 

There is a demo project. There will be better docs here as the extension matures.

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Better docs and examples

## Acknowledgments

This was written by referring a lot to [GodotVR](https://github.com/GodotVR) code and reading 
[Godot's](https://github.com/godotengine/godot) source code. 
