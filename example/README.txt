TiltFiveGodot is GDNative extension for the Godot engine to connect to the Tilt Five(https://www.tiltfive.com/) 
system. It implements Godot's ARVRinterface and has GDNative class called TiltFiveManager for scripts to connect 
glasses and handle connection events.

Basic usage

See https://patrickdown.github.io/tilt-five-godot/Tilt%20Five%20Godot.html for more comprehensive
instructions

1) Install using Godot's `install` option in the AssetLib tab or by unzipping this archive 
into the root of your Godot project. 

2) In Godot's project setting add `addons\tilt-five\TiltFiveManager` into the `Autoload` tab.

3) From `addons\tilt-five\scenes` load the `t5-scene` or `TiltFiveRig` scenes and run. 

4) From here you should be able to follow documentation for usage of Godot's AR/VR system.
