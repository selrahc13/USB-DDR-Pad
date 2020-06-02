#USB DDR Pad
Custom software for a custom-build dance pad based on the DanceForce v3 design by Promit Roy (https://ventspace.wordpress.com/2018/04/09/danceforce-v3-diy-dance-pad-controller/)

I initially used Promit Roy's sample code but eventually ended up rewriting it all from the ground up.

I've only run this code on a SparkFun Pro Micro but it shouldn't be too difficult to port to any Arduino-compatible board
using the Atmega 32u4 chip.

Support is included for generic HID joystick (6 buttons) or for the adventurous, you can enable XInput code to emulate an XBox 360 controller

HID Joystick support requires libraries from the following repositories to be installed:
https://github.com/dmadison/HID_Buttons
https://github.com/MHeironimus/ArduinoJoystickLibrary

XInput is a little more complex, but at its core requires you to install the XInput library as well as XInput AVR board defitions.
https://github.com/dmadison/ArduinoXInput

Many options are configurable through code; have a look. It's not the cleanest code, I just wanted to put together something that worked well enough for my purposes.
