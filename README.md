USB Dance Pad - for Arduino ATmega 32u4 boards such as Leonardo

I had originally used example code provided by Promit Roy to power my board, but quickly
rewrote the code entirely. I added a debounce routine, and a startup routine to calibrate
button thresholds automatically. I based my board design on the DanceForce v3: https://ventspace.wordpress.com/2018/04/09/danceforce-v3-diy-dance-pad-controller/

USB HID and XInput are both supported (not at the same time!)

For HID Joystick support, libraries from these repositories must be installed:
https://github.com/MHeironimus/ArduinoJoystickLibrary
https://github.com/dmadison/HID_Buttons

For XInput support, which is more tricky, install the XInput library and a compatible board file from:
https://github.com/dmadison/ArduinoXInput

It's very configurable, and pretty ugly, but it works for my purposes.