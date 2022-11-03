# TiltFiveGodot

TiltFiveGodot is **GDNative** extension for the Godot engine to connect to the [Tilt Five](https://www.tiltfive.com/) 
system. It implements Godot's **ARVRinterface** and has GDNative class called TiltFiveManager for scripts to connect 
glasses and handle connection events.

This extension is in it's early days and is probably buggy and subject to changes in interface.

## Build

Currently only Windows 10/11 is supported because that is the only platform supported by Tilt Five. T5 linux support
is supposed to come at some point in the future and support for that platform will revisted when it becomes available. 
This project uses the same [SCons](https://scons.org/) build system used by Godot project. It might be useful to refer 
to Godot's own [documentation](https://docs.godotengine.org/en/stable/tutorials/scripting/gdnative/index.html) 
about building extensions.

## Using 

There is a demo project. There will be better docs here as the extension matures.

## Dependencies

- Uses the godot-cpp headers
- Uses the Tilt Five NDK

## TODO

- Wand tracking
- Wand input
- Move connection management to a background thread
- Better docs and examples

